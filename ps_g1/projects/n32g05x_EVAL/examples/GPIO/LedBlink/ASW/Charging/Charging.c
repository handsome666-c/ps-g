#include "charging.h"

/**********充电初始化***************/
charging_event_struct_t chargingevt;
static charging_info_t st_charging_info;
static charging_state_machine_params_t st_charging_state_machine_params = {.last_state = E_STATE_CHG_POST, .curr_state = E_STATE_CHG_POST};
static diode_result_t st_diode_result;
static charging_session_t st_charging_session = {
    .start_charging_count = 0,
    .over_current_count = 0,
    .is_charging_session_active = false
};

static void APP_CHG_SetFault(fault_charging_type_t faultType);
static void APP_CHG_ClearAllFaults(void);
static void APP_CHG_ChargingSessionStart(void);
static void APP_CHG_ChargingSessionStop(void);
static void APP_CHG_SetDiodetatus(void);

/**********放电初始化***************/
discharging_event_struct_t dischargingevt;
static discharging_info_t st_discharging_info;
static discharging_state_machine_params_t st_discharging_state_machine_params = {.last_state = E_STATE_DISCHG_POST, .curr_state = E_STATE_DISCHG_POST};

static void APP_DISCHG_SetFault(fault_discharging_type_t faultType);
static void APP_DISCHG_ClearAllFaults(void);
/**********其他需求初始化***************/


/****************************************充电状态机****************************************/
static void APP_FSM_TransitionChargingToFaultOnFault(const charging_event_struct_t* evt) //充电中-故障
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0);
    Relay_hold_off(); //立即关闭继电器
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type);  //设置故障码
}

static void APP_FSM_TransitionChargingToWarningOnWarning(const charging_event_struct_t* evt)
{
    rt_kprintf("DBG_INFO, CHG, ChargingToWarningOnWarning\r\n");
}

static void APP_FSM_TransitionChargingToPPauseOnProtectionPause(const charging_event_struct_t* evt)//充电中-保护暂停
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0); //CP切DC
    dat.type = (fault_charging_type_t)evt->data;
    if(dat.type == E_CHARGING_FAULT_CP_ABNORMAL)
    {
        Relay_hold_off(); //立即关闭继电器
    }else if(dat.type == E_CHARGING_FAULT_OVER_CURRENT)
    {
        Relay_delay_hold_off(15); //延时1.5s关闭继电器
    }else{
        Relay_delay_hold_off(60); //延时6s关闭继电器
    }
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionPostToFaultOnPostFailure(const charging_event_struct_t* evt) //开机自检-自检失败
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;  //事件数据中获取故障状态
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionIdleToFaultOnFault(const charging_event_struct_t* evt) //空闲-故障
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionIdleToWarningOnWarning(const charging_event_struct_t* evt) //空闲-告警
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionIdleToPPauseOnProtectionPause(const charging_event_struct_t* evt)// 空闲-保护暂停
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionConnectedToWarningOnWarning(const charging_event_struct_t* evt) //插枪-警告
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionConnectedToFaultOnFault(const charging_event_struct_t* evt)
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionConnectedToPauseOnProtectionPause(const charging_event_struct_t* evt)//插枪-保护暂停
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_FSM_TransitionReadyToFaultOnFault(const charging_event_struct_t* evt) //就绪-故障
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0); //CP关闭PWM
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionReadyToWarningOnWarning(const charging_event_struct_t* evt) //就绪-告警
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0); //CP切DC
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionReadyToPPauseOnProtectionPause(const charging_event_struct_t* evt) //就绪-保护暂停
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0); //CP切DC
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionDiodecheckToFaultOnDiodecheckComplete(const charging_event_struct_t* evt) //二极管检测-故障-就绪
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    if(E_CHARGING_FAULT_DIODE_CHECK ==  dat.type)
    {
        st_diode_result = DIODE_RESULT_NO_DIODE; //无二极管
    }
    else
    {
        st_diode_result = DIODE_RESULT_NORMAL;    //有二极管
    }
    APP_CHG_ChargingSessionStart();
}

static void APP_FSM_TransitionVPauseToFaultOnFault(const charging_event_struct_t* evt) //车端暂停-故障, 无应用
{
    rt_kprintf("DBG_INFO, CHG, VPauseToFaultOnFault\r\n");
}

static void APP_FSM_TransitionVPauseToWarningOnWarning(const charging_event_struct_t* evt)
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0); //CP切DC
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionVPauseToPPauseOnProtectionPause(const charging_event_struct_t* evt) //车端暂停-保护暂停
{
    event_fault_charging_data_t dat;
    APP_CP_SetState(0); //CP切DC
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionPPauseToFaultOnFault(const charging_event_struct_t* evt) //保护暂停-故障,无应用场景
{
    rt_kprintf("DBG_INFO, CHG, PPauseToFaultOnFault\r\n");
}

static void APP_FSM_TransitionPPauseToWarningOnWarning(const charging_event_struct_t* evt)
{
    rt_kprintf("DBG_INFO, CHG, PPauseToWarningOnWarning\r\n");
}

static void APP_FSM_TransitionFaultToIdleOnSystemReset(const charging_event_struct_t* evt) //故障-系统复位 无应用
{
    NVIC_SystemReset(); //NVIC复位
}

static void APP_FSM_TransitionWarningToIdleOnWarningRecovery(const charging_event_struct_t* evt) //告警-空闲、插枪
{
    APP_CHG_ClearAllFaults();
    //告警恢复的时候, 根据cp当前电压, 决定跳转到空闲或者插枪状态
    if(E_STATE_UNPLUGGED == APP_CP_GetCurrentStatus()) //未连接
    {
        //本身就是未连接状态, 不处理
//        APP_CHG_EventPubTopic(E_EVENT_CHG_PLUG_OUT);
    }else if(E_STATE_PLUGGED == APP_CP_GetCurrentStatus()) //车辆连接
    {
        APP_CHG_EventPubTopic(E_EVENT_CHG_PLUG_IN);
    }
    else{
        //未知状态  //如果此时cp是未知状态, 这里不进行处理, 跳转到空闲状态后, 有cp模块触发故障
    }
}

static void APP_FSM_TransitionWarningToCONNECTEDOnCONNECTED(const charging_event_struct_t* evt)//告警-插枪
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    if(dat.type == E_CHARGING_FAULT_GROUND_FAULT)
    {
        APP_CHG_ClearAllFaults();
    }
    APP_CHG_EventPubTopic(E_EVENT_CHG_NOAUTH_START); //无需鉴权启充事件
}

static void APP_FSM_TransitionPPauseToReadyOnProtectionRecovery(const charging_event_struct_t* evt)//保护暂停-恢复
{
    APP_CHG_ClearAllFaults();
    APP_CP_SetState(1); //CP打开PWM
}

static void APP_FSM_TransitionPostToIdleOnPostSuccessIdle(const charging_event_struct_t* evt)//开机自检-空闲
{
    rt_kprintf("DBG_INFO, CHG, PostToIdleOnPostSuccessIdle\r\n");
}

static void APP_FSM_TransitionPostToConnectedOnPostSuccessConnected(const charging_event_struct_t* evt)//开机自检-插枪
{
    APP_CHG_EventPubTopic(E_EVENT_CHG_NOAUTH_START); //无需鉴权启充事件
}

static void APP_FSM_TransitionIdleToConnectedOnPlugIn(const charging_event_struct_t* evt) //空闲-插枪
{
    APP_CHG_SetDiodetatus();//二极管状态设置为正常
    APP_CHG_EventPubTopic(E_EVENT_CHG_NOAUTH_START); //无需鉴权启充事件
}

static void APP_FSM_TransitionConnectedToPrecheckOnNoAuthStart(const charging_event_struct_t* evt) //插枪-充电预检
{
    PreCheck_Charging_Start(&Pre_Manager); //启动预检
}

static void APP_FSM_TransitionConnectedToIdleOnPlugOut(const charging_event_struct_t* evt)//插枪-空闲
{
    rt_kprintf("DBG_INFO, CHG, ConnectedToIdleOnPlugOut\r\n");
}

static void APP_FSM_TransitionPrecheckToDiodecheckOnPrecheckSuccess(const charging_event_struct_t* evt) //充电预检-二极管检测
{
    APP_CP_SetState(1); //CP打开PWM
    if(E_STATE_PLUGGED == APP_CP_GetCurrentStatus())
    {
        APP_DiodeCheck_Start(); //启动二极管检测
    }
}

static void APP_FSM_TransitionPrecheckToIdleOnIdle(const charging_event_struct_t* evt) //充电预检-空闲
{
    APP_CP_SetState(0); //CP打开PWM
}

static void APP_FSM_TransitionPrecheckToFaultOnPrecheckFailure(const charging_event_struct_t* evt) //充电预检-故障
{
    event_fault_charging_data_t dat;
    dat.type = (fault_charging_type_t)evt->data;
    APP_CHG_SetFault(dat.type); //设置故障码
}

static void APP_FSM_TransitionDiodecheckToIdleOnIdle(const charging_event_struct_t* evt) //二极管检测-空闲
{
    APP_CP_SetState(0); //CP打开PWM
}

static void APP_FSM_TransitionPrecheckToChargingOnVehicleStart(const charging_event_struct_t* evt) //二极管检测-充电中
{
    Relay_hold_on();  //打开继电器
}

static void APP_FSM_TransitionDiodecheckToReadyOnDiodecheckComplete(const charging_event_struct_t* evt)//二极管检测=ok-就绪
{
    st_diode_result = DIODE_RESULT_NORMAL; //有二极管
    APP_CHG_ChargingSessionStart();
}

static void APP_FSM_TransitionReadyToIdleOnPlugOut(const charging_event_struct_t* evt) //就绪-拔枪
{
    APP_CP_SetState(0); //CP关闭PWM
}

static void APP_FSM_TransitionReadyToChargingOnVehicleStart(const charging_event_struct_t* evt) //就绪-启冲
{
    Relay_hold_on(); //闭合继电器
}

static void APP_FSM_TransitionChargingToIdleOnPlugOut(const charging_event_struct_t* evt) //充电中-空闲
{
    APP_CP_SetState(0); //CP关闭PWM
    Relay_hold_off(); //立即关闭继电器
    APP_CHG_ChargingSessionStop();
}

static void APP_FSM_TransitionChargingToVPauseOnVehiclePause(const charging_event_struct_t* evt) //充电中-车端暂停
{
    Relay_hold_off(); //立即关闭继电器
    st_charging_session.start_charging_count++;
    if(st_charging_session.start_charging_count >= MAX_START_CHARGING_COUNT)
    {
        APP_CHG_EventPubTopic(E_EVENT_CHG_VEHICLE_EXCESSIVE_START_COUNT);
    }
}

static void APP_FSM_TransitionVPauseToChargingOnVehicleStart(const charging_event_struct_t* evt) //车端暂停-充电中
{
    Relay_hold_on();  //闭合继电器
}

static void APP_FSM_TransitionVPauseToConnectedOnVehicleExcessiveStartCount(const charging_event_struct_t* evt) //车端暂停-大于六次-插枪
{
    APP_CP_SetState(0);
    APP_CHG_ChargingSessionStop();
}

static void APP_FSM_TransitionVPauseToIdleOnPlugOut(const charging_event_struct_t* evt)//车端暂停-空闲
{
    APP_CP_SetState(0);
    APP_CHG_ChargingSessionStop();
}

static void APP_FSM_TransitionPPauseToIdleOnPlugOut(const charging_event_struct_t* evt) //保护暂停-空闲
{
    APP_CHG_ChargingSessionStop();
}

static const charging_state_transition_t st_charging_transition_table[] = {

/************************************************故障保护-优先级最高-放在前面************************************************/
    {E_STATE_CHG_CHARGING, E_EVENT_CHG_FAULT, E_STATE_CHG_FAULT, APP_FSM_TransitionChargingToFaultOnFault},
    {E_STATE_CHG_CHARGING, E_EVENT_CHG_WARNING, E_STATE_CHG_WARNING, APP_FSM_TransitionChargingToWarningOnWarning},
    {E_STATE_CHG_CHARGING, E_EVENT_CHG_PROTECTION_PAUSE, E_STATE_CHG_PPAUSE, APP_FSM_TransitionChargingToPPauseOnProtectionPause},
    {E_STATE_CHG_POST, E_EVENT_CHG_POST_FAILURE, E_STATE_CHG_FAULT, APP_FSM_TransitionPostToFaultOnPostFailure},
    {E_STATE_CHG_IDLE, E_EVENT_CHG_FAULT, E_STATE_CHG_FAULT, APP_FSM_TransitionIdleToFaultOnFault},
    {E_STATE_CHG_IDLE, E_EVENT_CHG_WARNING, E_STATE_CHG_WARNING, APP_FSM_TransitionIdleToWarningOnWarning},
    {E_STATE_CHG_IDLE, E_EVENT_CHG_PROTECTION_PAUSE, E_STATE_CHG_PPAUSE, APP_FSM_TransitionIdleToPPauseOnProtectionPause},
    {E_STATE_CHG_CONNECTED, E_EVENT_CHG_FAULT, E_STATE_CHG_FAULT, APP_FSM_TransitionConnectedToFaultOnFault},
    {E_STATE_CHG_CONNECTED, E_EVENT_CHG_WARNING, E_STATE_CHG_WARNING, APP_FSM_TransitionConnectedToWarningOnWarning},
    {E_STATE_CHG_CONNECTED, E_EVENT_CHG_PROTECTION_PAUSE, E_STATE_CHG_PPAUSE, APP_FSM_TransitionConnectedToPauseOnProtectionPause},
    {E_STATE_CHG_READY, E_EVENT_CHG_FAULT, E_STATE_CHG_FAULT, APP_FSM_TransitionReadyToFaultOnFault},
    {E_STATE_CHG_READY, E_EVENT_CHG_WARNING, E_STATE_CHG_WARNING, APP_FSM_TransitionReadyToWarningOnWarning},
    {E_STATE_CHG_READY, E_EVENT_CHG_PROTECTION_PAUSE, E_STATE_CHG_PPAUSE, APP_FSM_TransitionReadyToPPauseOnProtectionPause},
    {E_STATE_CHG_DIODECHECK, E_EVENT_CHG_DIODECHECK_FAILURE, E_STATE_CHG_READY, APP_FSM_TransitionDiodecheckToFaultOnDiodecheckComplete},
    {E_STATE_CHG_VPAUSE, E_EVENT_CHG_FAULT, E_STATE_CHG_FAULT, APP_FSM_TransitionVPauseToFaultOnFault},
    {E_STATE_CHG_VPAUSE, E_EVENT_CHG_WARNING, E_STATE_CHG_WARNING, APP_FSM_TransitionVPauseToWarningOnWarning},
    {E_STATE_CHG_VPAUSE, E_EVENT_CHG_PROTECTION_PAUSE, E_STATE_CHG_PPAUSE, APP_FSM_TransitionVPauseToPPauseOnProtectionPause},
    {E_STATE_CHG_PPAUSE, E_EVENT_CHG_FAULT, E_STATE_CHG_FAULT, APP_FSM_TransitionPPauseToFaultOnFault},
    {E_STATE_CHG_PPAUSE, E_EVENT_CHG_WARNING, E_STATE_CHG_WARNING, APP_FSM_TransitionPPauseToWarningOnWarning},
    {E_STATE_CHG_FAULT, E_EVENT_CHG_SYSTEM_RESET, E_STATE_CHG_IDLE, APP_FSM_TransitionFaultToIdleOnSystemReset},
    {E_STATE_CHG_WARNING, E_EVENT_CHG_PLUG_IN, E_STATE_CHG_CONNECTED, APP_FSM_TransitionWarningToCONNECTEDOnCONNECTED},
    {E_STATE_CHG_WARNING, E_EVENT_CHG_WARNING_RECOVERY, E_STATE_CHG_IDLE, APP_FSM_TransitionWarningToIdleOnWarningRecovery},
    {E_STATE_CHG_PPAUSE, E_EVENT_CHG_PROTECTION_RECOVERY, E_STATE_CHG_READY, APP_FSM_TransitionPPauseToReadyOnProtectionRecovery},

/********************************************************************************************************/
    {E_STATE_CHG_POST, E_EVENT_CHG_POST_SUCCESS_IDLE, E_STATE_CHG_IDLE, APP_FSM_TransitionPostToIdleOnPostSuccessIdle},
    {E_STATE_CHG_POST, E_EVENT_CHG_POST_SUCCESS_CONNECTED, E_STATE_CHG_CONNECTED, APP_FSM_TransitionPostToConnectedOnPostSuccessConnected},
    {E_STATE_CHG_IDLE, E_EVENT_CHG_PLUG_IN, E_STATE_CHG_CONNECTED, APP_FSM_TransitionIdleToConnectedOnPlugIn},
    {E_STATE_CHG_CONNECTED, E_EVENT_CHG_NOAUTH_START, E_STATE_CHG_PRECHECK, APP_FSM_TransitionConnectedToPrecheckOnNoAuthStart},
    {E_STATE_CHG_CONNECTED, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionConnectedToIdleOnPlugOut},
    {E_STATE_CHG_PRECHECK, E_EVENT_CHG_PRECHECK_SUCCESS, E_STATE_CHG_DIODECHECK, APP_FSM_TransitionPrecheckToDiodecheckOnPrecheckSuccess},
    {E_STATE_CHG_PRECHECK, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionPrecheckToIdleOnIdle},
    {E_STATE_CHG_PRECHECK, E_EVENT_CHG_PRECHECK_FAILURE, E_STATE_CHG_FAULT, APP_FSM_TransitionPrecheckToFaultOnPrecheckFailure},
    {E_STATE_CHG_DIODECHECK, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionDiodecheckToIdleOnIdle},
    {E_STATE_CHG_DIODECHECK, E_EVENT_CHG_VEHICLE_START, E_STATE_CHG_CHARGING, APP_FSM_TransitionPrecheckToChargingOnVehicleStart},
    {E_STATE_CHG_DIODECHECK, E_EVENT_CHG_DIODECHECK_COMPLETE, E_STATE_CHG_READY, APP_FSM_TransitionDiodecheckToReadyOnDiodecheckComplete},
    {E_STATE_CHG_READY, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionReadyToIdleOnPlugOut},
    {E_STATE_CHG_READY, E_EVENT_CHG_VEHICLE_START, E_STATE_CHG_CHARGING, APP_FSM_TransitionReadyToChargingOnVehicleStart},
    {E_STATE_CHG_CHARGING, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionChargingToIdleOnPlugOut},
    {E_STATE_CHG_CHARGING, E_EVENT_CHG_VEHICLE_PAUSE, E_STATE_CHG_VPAUSE, APP_FSM_TransitionChargingToVPauseOnVehiclePause},
    {E_STATE_CHG_VPAUSE, E_EVENT_CHG_VEHICLE_START, E_STATE_CHG_CHARGING, APP_FSM_TransitionVPauseToChargingOnVehicleStart},
    {E_STATE_CHG_VPAUSE, E_EVENT_CHG_VEHICLE_EXCESSIVE_START_COUNT, E_STATE_CHG_CONNECTED, APP_FSM_TransitionVPauseToConnectedOnVehicleExcessiveStartCount},
    {E_STATE_CHG_VPAUSE, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionVPauseToIdleOnPlugOut},
    {E_STATE_CHG_PPAUSE, E_EVENT_CHG_PLUG_OUT, E_STATE_CHG_IDLE, APP_FSM_TransitionPPauseToIdleOnPlugOut},
};
static int APP_CHG_EventPubRet(uint16_t topic, uint16_t data)
{
    chargingevt.topic = topic;
    if(data != NULL){
        chargingevt.data = data;
    }else{
        chargingevt.data = NULL;
    }
    return E_ChargeEventRun_OK;
}

int APP_CHG_EventPubTopic(uint16_t topic)
{
    int ret = APP_CHG_EventPubRet(topic, NULL);
    return ret;
}

int APP_CHG_EventPub(uint16_t topic, uint16_t data)
{
    int ret = APP_CHG_EventPubRet(topic, data);
    return ret;
}

int APP_CHG_StateMachineDispatch(void)
{
    for(int i = 0; i < (sizeof(st_charging_transition_table)/sizeof(charging_state_transition_t)); i++)
    {
        const charging_state_transition_t* t = &st_charging_transition_table[i];
        if((t->current_state == st_charging_state_machine_params.curr_state) && (t->event == chargingevt.topic))
        {
            st_charging_state_machine_params.last_state = st_charging_state_machine_params.curr_state;
            st_charging_state_machine_params.curr_state = t->next_state;
            if(t->action){
                t->action(&chargingevt);
            }
            break;
        }
    }
    return E_ChargeStateMachineRun_OK;
}

void APP_CHG_StateMachineInit(void)
{
    st_charging_state_machine_params.last_state = E_STATE_CHG_POST;
    st_charging_state_machine_params.curr_state = E_STATE_CHG_POST;
}

charging_state_t APP_CHG_GetChargeStatus(void)
{
    return (st_charging_state_machine_params.curr_state);
}

uint8_t APP_CHG_GetStatusLED(void) //灯获取充电状态函数
{
    uint8_t state = 0xFF;
    switch(st_charging_state_machine_params.curr_state)
    {
        case E_STATE_CHG_POST: state = 0; break;      //开机自检，蓝-绿-蓝-红-灭 顺序闪烁（1s）
        case E_STATE_CHG_IDLE: state = 1; break;      //空闲，蓝常亮
        case E_STATE_CHG_CONNECTED: state = 2; break; //插枪，连接，绿闪烁
        case E_STATE_CHG_PRECHECK: state = 2; break;  // 预检，绿闪烁
        case E_STATE_CHG_DIODECHECK: state = 2; break;// 二极管检测，绿闪烁
        case E_STATE_CHG_READY: state = 2; break;     //就绪，绿闪烁
        case E_STATE_CHG_CHARGING: state = 3; break;  //充电，绿色呼吸
        case E_STATE_CHG_VPAUSE: state = 4; break;    //暂停，绿常量
        case E_STATE_CHG_PPAUSE: state = 5; break;    //保护暂停，红亮n次停3s循环
//        case E_STATE_CHG_WARNING: state = 5; break;   //警告
        case E_STATE_CHG_FAULT: state = 5; break;     //故障，红亮n次停3s循环
        default: state = 5;  break;
    }
    return state;
}

diode_result_t APP_CHG_GetDiodeStatus(void)
{
    return st_diode_result;
}

void APP_CHG_SetDiodetatus(void)
{
    st_diode_result = DIODE_RESULT_NORMAL;
}

void APP_CHG_SetFault(fault_charging_type_t faultType)
{
    if (faultType < E_CHARGING_FAULT_TYPE_MAX){
        st_charging_info.errorCode = faultType;
    }
}

uint32_t APP_CHG_GetFault(void)
{
    return (st_charging_info.errorCode);
}

// 清除所有故障
void APP_CHG_ClearAllFaults(void)
{
    st_charging_info.errorCode = 0;  // 直接清零，清除所有故障位
}

// 判断特定故障是否存在
bool APP_CHG_IsFaultPresent(fault_charging_type_t faultType) {
    if (faultType >= E_CHARGING_FAULT_TYPE_MAX) {
        return false;
    }
    if(faultType == st_charging_info.errorCode) {
        return true;
    }else {
        return false;
    }
}

// 判断是否有任何故障
bool APP_CHG_IsAnyFaultPresent(void) {
    return APP_CHG_GetFault() != 0;
}

void APP_CHG_ChargingSessionStart(void)
{

}

void APP_CHG_ChargingSessionStop(void)
{
    st_charging_session.start_charging_count = 0;
    st_charging_session.over_current_count = 0;
    st_charging_session.is_charging_session_active = false;
    APP_DIAG_CHG_CurReset(&Diagnostic_Manager);
}

/************************************************放电状态机************************************************/
static void APP_Dis_Transtion_PostToPrecheckonprecheck(const discharging_event_struct_t* evt) //开机自检-预检
{
    PreCheck_Discharging_Start(&Pre_Manager);  //启动预检
}

static void APP_Dis_Transtion_PostToPostFailure(const discharging_event_struct_t* evt) //开机自检- 故障
{
    event_fault_discharging_data_t dat;
    dat.type = (fault_discharging_type_t)evt->data;  //事件数据中获取故障状态
    APP_DISCHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_Dis_Transtion_PrecheckToDischarging(const discharging_event_struct_t* evt) //预检 - 放电中
{
    Relay_hold_on(); //闭合继电器
}

static void APP_Dis_Transtion_PrecheckToPrecheckwarning(const discharging_event_struct_t* evt) //预检-告警
{
    event_fault_discharging_data_t dat;
    dat.type = (fault_discharging_type_t)evt->data;  //事件数据中获取故障状态
    APP_DISCHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_Dis_Transtion_WarningTodischargingondischarging(const discharging_event_struct_t* evt)  //告警恢复
{
    APP_DISCHG_ClearAllFaults();
    Relay_hold_on(); //闭合继电器
}

static void APP_Dis_Transtion_DischargingToFailure(const discharging_event_struct_t* evt)
{
    event_fault_discharging_data_t dat;
    Relay_hold_off(); //立即关闭继电器
    dat.type = (fault_discharging_type_t)evt->data;  //事件数据中获取故障状态
    APP_DISCHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_Dis_Transtion_DischargingToPPause(const discharging_event_struct_t* evt)
{
    event_fault_discharging_data_t dat;
    dat.type = (fault_discharging_type_t)evt->data;  //事件数据中获取故障状态
    if(dat.type == E_DISCHARGING_FAULT_OVER_CURRENT)
    {
        Relay_delay_hold_off(15); //延时1.5s关闭继电器
    }else{
        Relay_delay_hold_off(60); //延时6s关闭继电器
    }
    APP_DISCHG_SetFault(dat.type); //设置故障码-灯语使用
}

static void APP_Dis_Transtion_PPauseToDischarging(const discharging_event_struct_t* evt)
{
    APP_DISCHG_ClearAllFaults();
    Relay_hold_on(); //闭合继电器
}

static void APP_Dis_Transtion_FailureToDischarging(const discharging_event_struct_t* evt)
{
    APP_DISCHG_ClearAllFaults();
    Relay_hold_on(); //闭合继电器
}

static const discharging_state_transition_t st_discharging_transition_table[] = {
    {E_STATE_DISCHG_POST, E_EVENT_DISCHG_POST_SUCCESS, E_STATE_DISCHG_PRECHECK, APP_Dis_Transtion_PostToPrecheckonprecheck}, //开机放电自检--开机放电自检成功--放电预检
    {E_STATE_DISCHG_POST, E_EVENT_DISCHG_POST_FAILURE, E_STATE_DISCHG_PPAUSE, APP_Dis_Transtion_PostToPostFailure},//开机放电自检--开机放电自检失败--故障
    {E_STATE_DISCHG_PRECHECK, E_EVENT_DISCHG_PRECHECK_SUCCESS, E_STATE_DISCHG_DISCHARGING, APP_Dis_Transtion_PrecheckToDischarging}, //放电预检--放电预检成功--放电中,放电预检只检测继电器粘连，通过则闭合
    {E_STATE_DISCHG_PRECHECK, E_EVENT_DISCHG_PRECHECK_FAILURE, E_STATE_DISCHG_WARNING, APP_Dis_Transtion_PrecheckToPrecheckwarning},//放电预检--放电预检失败--告警
    {E_STATE_DISCHG_WARNING, E_EVENT_DISCHG_WARNING_RECOVERY, E_STATE_DISCHG_DISCHARGING, APP_Dis_Transtion_WarningTodischargingondischarging},//告警--告警恢复--放电中
    {E_STATE_DISCHG_DISCHARGING, E_EVENT_DISCHG_FAULT, E_STATE_DISCHG_FAULT, APP_Dis_Transtion_DischargingToFailure},//放电中--故障--故障
    {E_STATE_DISCHG_DISCHARGING, E_EVENT_DISCHG_PROTECTION_PAUSE, E_STATE_DISCHG_PPAUSE, APP_Dis_Transtion_DischargingToPPause},//放电中--保护暂停--保护暂停
    {E_STATE_DISCHG_PPAUSE, E_EVENT_DISCHG_PROTECTION_RECOVERY, E_STATE_DISCHG_DISCHARGING, APP_Dis_Transtion_PPauseToDischarging},//保护暂停--保护恢复--放电中
    {E_STATE_DISCHG_FAULT,  E_EVENT_DISCHG_SYSTEM_RESET, E_STATE_DISCHG_DISCHARGING, APP_Dis_Transtion_FailureToDischarging}, //放电故障--系统复位--放电中
};

static int APP_DISCHG_EventPubRet(uint16_t topic, uint16_t data)
{
    dischargingevt.topic = topic;
    if(data != NULL) {
        dischargingevt.data = data;
    } else {
        dischargingevt.data = NULL;
    }
    return E_ChargeEventRun_OK;
}

int APP_DISCHG_EventPubTopic(uint16_t topic)
{
    int ret = APP_DISCHG_EventPubRet(topic, NULL);
    return ret;
}

int APP_DISCHG_EventPub(uint16_t topic, uint16_t data)
{
    int ret = APP_DISCHG_EventPubRet(topic, data);
    return ret;
}

int APP_DISCHG_StateMachineDispatch(void)
{
    for(int i = 0; i < (sizeof(st_discharging_transition_table)/sizeof(discharging_state_transition_t)); i++)
    {
        const discharging_state_transition_t* t = &st_discharging_transition_table[i];
        if((t->current_state == st_discharging_state_machine_params.curr_state) && (t->event == dischargingevt.topic))
        {
            st_discharging_state_machine_params.last_state = st_discharging_state_machine_params.curr_state;
            st_discharging_state_machine_params.curr_state = t->next_state;
            if(t->action)
            {
                t->action(&dischargingevt);
            }
            break;
        }
    }
    return E_ChargeStateMachineRun_OK;
}

void APP_DISCHG_StateMachineInit(void)
{
    st_discharging_state_machine_params.last_state = E_STATE_DISCHG_POST;
    st_discharging_state_machine_params.curr_state = E_STATE_DISCHG_POST;
}

discharging_state_t APP_DISCHG_GetChargeStatus(void)
{
    return (st_discharging_state_machine_params.curr_state);
}

uint8_t APP_DISCHG_GetStatusLED(void) //灯获取充电状态函数
{
    uint8_t state = 0xFF;
    switch(st_discharging_state_machine_params.curr_state)
    {
        case E_STATE_DISCHG_POST: state = 0; break;      //开机自检，蓝-绿-蓝-红-灭 顺序闪烁（1s）
        case E_STATE_DISCHG_PRECHECK: state = 6; break;  //放电等待，黄常亮
        case E_STATE_DISCHG_DISCHARGING: state = 7; break;//正常放电，黄闪烁
        case E_STATE_DISCHG_PPAUSE: state = 8; break;    // 保护暂停
        case E_STATE_DISCHG_WARNING: state = 8; break;   // 告警
        case E_STATE_DISCHG_FAULT: state = 8; break;     // 故障
        default: state = 8; break;
    }
    return state;
}

void APP_DISCHG_SetFault(fault_discharging_type_t faultType)
{
    if (faultType < E_DISCHARGING_FAULT_TYPE_MAX)
    {
        st_discharging_info.errorCode = faultType; // 设置故障位
    }
}

uint32_t APP_DISCHG_GetFault(void)
{
    return st_discharging_info.errorCode;
}

// 清除所有故障
void APP_DISCHG_ClearAllFaults(void)
{
    st_discharging_info.errorCode = 0; // 直接清零，清除所有故障位
}

// 判断特定故障是否存在
bool APP_DISCHG_IsFaultPresent(fault_discharging_type_t faultType) {
    if (faultType >= E_DISCHARGING_FAULT_TYPE_MAX) {
        return false;
    }
    if(faultType == st_discharging_info.errorCode)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// 判断是否有任何故障
bool APP_DISCHG_IsAnyFaultPresent(void) {
    return APP_DISCHG_GetFault() != 0;
}

void APP_DISCHG_SessionStart(void)
{

}

void APP_DISCHG_SessionStop(void)
{
    APP_DIAG_DISCHG_CurReset(&Diagnostic_Manager);
}
