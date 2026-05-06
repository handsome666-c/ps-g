#include "cp.h"

cp_state_machine_t st_cp_state_machine = {.current_state = E_STATE_UNPLUGGED};
Adapter_state_machine_t st_Adapter_state_machine;
static cp_filter_t g_cp_filter;
/**
 * @brief 判断电压是否处于异常区域（需要忽略或上报）
 */
static bool is_in_abnormal_zone(uint32_t voltage) {
    // 1. CP 小于 2V - 异常
    if (voltage < 200) {
        return true;
    }
    // 2. CP 大于 13.2V - 异常
    if (voltage > 1320) {
        return true;
    }
    // 3. CP 大于 7.2V 且 小于 7.8V - 未知过渡区域
    if (voltage > 720 && voltage < 780) {
        return true;
    }
    // 4. CP 大于 10.2V 且 小于 10.8V - 未知过渡区域
    if (voltage > 1020 && voltage < 1080) {
        return true;
    }
    return false;
}

/**
 * @brief 转接头上电默认初始化，2kw默认8A
 */
void Adapter_detection_current_Init(Adapter_state_machine_t *st_Adapter)
{
#if (PRODUCT_TYPE == TYPE_2KW)
    st_Adapter->Adapter_current = PRODUCT_MODE_POWER_LOW;        // 8A
    st_Adapter->Adapter_current_last = PRODUCT_MODE_POWER_LOW;    // 8A
#endif

#if (PRODUCT_TYPE == TYPE_3_5KW) // 区分3.5kW转接头
    if(RTE_Call_AdcM_Vlotage(ADCMCFG_NTC1_CH) > 2000)
    {
        st_Adapter->Adapter_current = PRODUCT_MODE_POWER_HIGH;    // 16A
        st_Adapter->Adapter_current_last = PRODUCT_MODE_POWER_HIGH;    // 16A
    }
    else
    {
        st_Adapter->Adapter_current = PRODUCT_MODE_POWER_LOW;    // 8A
        st_Adapter->Adapter_current_last = PRODUCT_MODE_POWER_LOW;    // 8A
    }
#endif
}

/**
 * @brief 3.5kW工作时，转接头诊断，周期30s循环检测，检测跳变控制CP更新，过流值更新
 */
void APP_Adapter_detection(Adapter_state_machine_t *st_Adapter) // 转接头状态检测
{
    st_Adapter->timer += 1; // 每5ms加1
    if(E_STATE_CHG_CHARGING != APP_CHG_GetChargeStatus()) // 只有在充电中检测
    {
        return;
    }

    if(st_Adapter->timer <= 6000) // 每30s检测一次
    {
        return;
    }

    if(RTE_Call_AdcM_Vlotage(ADCMCFG_NTC1_CH) > 2000)
    {
        st_Adapter->Adapter_current = PRODUCT_MODE_POWER_HIGH;
    }
    else
    {
        st_Adapter->Adapter_current = PRODUCT_MODE_POWER_LOW;
    }

    if(st_Adapter->Adapter_current_last != st_Adapter->Adapter_current)
    {
        APP_CP_SetState(1);
        APP_DIAG_CHG_CurProtector_Set(&Diagnostic_Manager);
    }

    st_Adapter->Adapter_current_last = st_Adapter->Adapter_current;
    st_Adapter->timer = 0;
}

/**
 * @brief 获取当前最大输出电流
 */
uint8_t Get_Adapter_detection_current(Adapter_state_machine_t *st_Adapter)
{
    return st_Adapter->Adapter_current;
}





/**
 * @brief APP CP设置输出状态
 */
void APP_CP_SetState(uint8_t pwm_active) // 0A, 1/0
{
    float required_duty = Get_Adapter_detection_current(& st_Adapter_state_machine)/0.6f; //8A/0.6 = 13.3%  16A/0.6 = 26.6%

    uint16_t duty_cycle = (uint16_t)(required_duty*10); //13.3*10 = 133(占空比)

    if(0 == pwm_active)
    {
        st_cp_state_machine.pwm_active = 0;
        RTE_Call_PwmIf_Set_Duty(TIM5, OC3, 1000); //DC直流输出持续高电平
    }
    else
    {
        st_cp_state_machine.pwm_active = 1;
        RTE_Call_PwmIf_Set_Duty(TIM5, OC3, duty_cycle);
    }
}
/**
 * @brief APP CP获取PWM状态
 */
static uint8_t APP_CP_GetPwmStatus(void)
{
    return st_cp_state_machine.pwm_active;
}

/**
 * @brief CP状态检测函数
 */
cp_voltage_state_t APP_CP_DetectCPState(uint32_t voltage, uint8_t pwm_active)
{
    if (voltage >= VOLT_12V_MIN && voltage <= VOLT_12V_MAX)
    {
        st_cp_state_machine.state_timer = 0;
        return pwm_active ? E_CP_12V_PWM : E_CP_12V_DC; //PWM_active (=1返回PWM)，(=0返回DC)
    }
    else if (voltage >= VOLT_9V_MIN && voltage <= VOLT_9V_MAX)
    {
        st_cp_state_machine.state_timer = 0;
        return pwm_active ? E_CP_9V_PWM : E_CP_9V_DC;
    }
    else if (voltage >= VOLT_6V_MIN && voltage <= VOLT_6V_MAX)
    {
        st_cp_state_machine.state_timer = 0;
        return pwm_active ? E_CP_6V_PWM : E_CP_6V_DC;
    }
    return E_CP_UNKNOWN;
}

/**
 * @brief 状态机处理核心
 */
static cp_event_t APP_CP_ProcessStateMachine(cp_state_machine_t *sm, cp_voltage_state_t cp_state)
{
    // 异常状态处理
    if(cp_state == E_CP_UNKNOWN)
    {
        sm->current_state = E_STATE_ERROR;
        return E_EVENT_ERROR;
    }

    // 状态转移逻辑
    switch (sm->current_state)
    {
    /*未连接状态*/
    case E_STATE_UNPLUGGED:
        if (cp_state == E_CP_9V_DC)
        {
            sm->current_state = E_STATE_PLUGGED;
            return E_EVENT_PLUGIN;
        }
        else if (cp_state == E_CP_6V_DC)
        {
            sm->current_state = E_STATE_PLUGGED;
            return E_EVENT_PLUGIN;
        }
        break;

    /*车辆已连接状态*/
    case E_STATE_PLUGGED:
        if (cp_state == E_CP_6V_DC)  // EV准备就绪 (6V直流)
        {
            sm->current_state = E_STATE_EV_READY;
            return E_EVENT_EV_READY;
        }
        else if(cp_state == E_CP_9V_PWM) // 充电设备准备就绪 (9V PWM)
        {
            sm->current_state = E_STATE_EVSE_READY;
            return E_EVENT_EVSE_READY;
        }
        else if(cp_state == E_CP_6V_PWM)  //cp保护暂停恢复增加
        {
            sm->current_state = E_STATE_CHARGING;
            return E_EVENT_CHARGE_START;
        }
        else if(cp_state == E_CP_12V_DC || cp_state == E_CP_12V_PWM) // 拔枪事件
        {
            sm->current_state = E_STATE_UNPLUGGED;
            return E_EVENT_PLUGOUT;
        }
        else
        {
        }
        break;

    //------------------------------------
    // EV准备就绪状态
    //------------------------------------
    case E_STATE_EV_READY:
        // 充电开始 (6V PWM)
        if (cp_state == E_CP_6V_PWM)
        {
            sm->current_state = E_STATE_CHARGING;
            return E_EVENT_CHARGE_START;
        }
        // 返回连接状态
        else if (cp_state == E_CP_9V_DC)
        {
            sm->current_state = E_STATE_PLUGGED;
            // 可选: 增加EV_NOT_READY事件
        }
        // 拔枪事件
        else if (cp_state == E_CP_12V_DC || cp_state == E_CP_12V_PWM)
        {
            sm->current_state = E_STATE_UNPLUGGED;
            return E_EVENT_PLUGOUT;
        }
        else
        {
        }
        break;

    //------------------------------------
    // 充电设备准备就绪状态
    //------------------------------------
    case E_STATE_EVSE_READY:
        // 充电开始 (6V PWM)
        if (cp_state == E_CP_6V_PWM)
        {
            sm->current_state = E_STATE_CHARGING;
            return E_EVENT_CHARGE_START;
        }
        // 返回连接状态
        else if (cp_state == E_CP_9V_DC)
        {
            sm->current_state = E_STATE_PLUGGED;
            // 可选: 增加EVSE_NOT_READY事件
        }
        // 拔枪事件
        else if (cp_state == E_CP_12V_DC || cp_state == E_CP_12V_PWM)
        {
            sm->current_state = E_STATE_UNPLUGGED;
            return E_EVENT_PLUGOUT;
        }
        else
        {
        }
        break;

    //------------------------------------
    // 充电中状态
    //------------------------------------
    case E_STATE_CHARGING:
        // 充电暂停 (9V PWM)
        if (cp_state == E_CP_9V_PWM)
        {
            sm->current_state = E_STATE_EVSE_READY;
            return E_EVENT_CHARGE_PAUSE;
        }
        // EV准备就绪 (6V直流)
        else if (cp_state == E_CP_6V_DC)
        {
            sm->current_state = E_STATE_EV_READY;
            // 可选: 增加EV_PAUSED事件
        }
        // 拔枪事件
        else if (cp_state == E_CP_12V_DC || cp_state == E_CP_12V_PWM)
        {
            sm->current_state = E_STATE_UNPLUGGED;
            return E_EVENT_PLUGOUT;
        }
        else
        {
        }
        break;

    //------------------------------------
    // 错误状态
    //------------------------------------
    case E_STATE_ERROR:
        if (cp_state == E_CP_12V_DC)
        {
            sm->current_state = E_STATE_UNPLUGGED;
        }
        else if(cp_state == E_CP_9V_DC)
        {
            sm->current_state = E_STATE_PLUGGED;
        }
        else if(cp_state == E_CP_6V_DC)
        {
            sm->current_state = E_STATE_PLUGGED;
        }
        else if(cp_state == E_CP_9V_PWM)
        {
            sm->current_state = E_STATE_PLUGGED;
        }
        else if(cp_state == E_CP_6V_PWM)
        {
            sm->current_state = E_STATE_PLUGGED;
        }
        else
        {
            sm->current_state = E_STATE_UNPLUGGED;
        }
        return E_EVENT_ERROR_RECOVERY;
    //    break;
    }

    return E_EVENT_NONE;
}

/**
 * @brief APP_CP_HandleChargerEvent
 */
static void APP_CP_HandleChargerEvent(cp_event_t event, cp_voltage_state_t cp_state)
{
    event_warning_charging_data_t st_event_warning_charging_data;
    event_warning_recovery_charging_data_t st_event_warning_recovery_charging_data;
    event_protection_pause_charging_data_t st_event_protection_pause_charging_data;
    event_protection_recovery_charging_data_t st_event_protection_recovery_charging_data;

    switch (event) {
    case E_EVENT_PLUGIN: // 执行插枪操作: 启动自检等
        rt_kprintf("CPE_EVENT_PLUG_IN\r\n");
        if(!APP_CHG_IsFaultPresent(E_CHARGING_FAULT_RELAY_STICK))
        {
            APP_CHG_EventPubTopic(E_EVENT_CHG_PLUG_IN);
        }
        break;

    case E_EVENT_PLUGOUT: // 执行拔枪操作: 停止充电、关闭继电器
        rt_kprintf("CPE_EVENT_PLUG_OUT\r\n");
        APP_CHG_EventPubTopic(E_EVENT_CHG_PLUG_OUT);
        break;

    case E_EVENT_EV_READY: // 车辆准备就绪
        rt_kprintf("CPE_EVENT_EV_READY\r\n");
        break;

    case E_EVENT_EVSE_READY: // 充电枪准备就绪
        rt_kprintf("CPE_EVENT_EVSE_READY\r\n");
        break;

    case E_EVENT_CHARGE_START:  // 启充操作: 闭合主继电器
        rt_kprintf("CPE_EVENT_CHARGE_START\r\n");
        APP_CHG_EventPubTopic(E_EVENT_CHG_VEHICLE_START);
        break;

    case E_EVENT_CHARGE_PAUSE:  // 暂停操作: 断开继电器
        rt_kprintf("CPE_EVENT_CHARGE_PAUSE\r\n");
        APP_CHG_EventPubTopic(E_EVENT_CHG_VEHICLE_PAUSE);
        break;

    case E_EVENT_ERROR: // 错误处理: 进入安全状态
        // 前置检查: 如果系统有其他更高优先级的故障, 且不是cp故障引起的
        if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_CP_ABNORMAL)) {
            return;  // 系统被其他故障锁定, 不处理接地故障
        }

        //CP保护在空闲、插枪进入告警
        if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus() || \
           (E_STATE_CHG_CONNECTED == APP_CHG_GetChargeStatus()))
        {
            rt_kprintf("DBG_INFO, CP, E_WARNING_CP_ABNORMAL\r\n");
            st_event_warning_charging_data.type = E_CHARGING_WARNING_CP_ABNORMAL;  //空闲或插枪状态下cp异常, 上报告警
            APP_CHG_EventPub(E_EVENT_CHG_WARNING, st_event_warning_charging_data.type);
        }
        else if(E_STATE_CHG_PRECHECK == APP_CHG_GetChargeStatus() || \
                (E_STATE_CHG_DIODECHECK == APP_CHG_GetChargeStatus()) ||\
                (E_STATE_CHG_READY == APP_CHG_GetChargeStatus()) ||\
                (E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) ||\
                (E_STATE_CHG_VPAUSE == APP_CHG_GetChargeStatus()))
        {
            rt_kprintf("DBG_INFO, CP, E_PROTECTION_PAUSE_CP_ABNORMAL\r\n");
            st_event_protection_pause_charging_data.type = E_CHARGING_PROTECTION_PAUSE_CP_ABNORMAL; //其他状态下cp异常, 上报保护暂停
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_PAUSE, st_event_protection_pause_charging_data.type);
        }
        else{
        }
        break;

    case E_EVENT_ERROR_RECOVERY: //cp保护恢复
        //当前有故障, 并且是cp故障才执行保护恢复操作
        if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_CP_ABNORMAL)) {
            return;
        }
        if(E_STATE_CHG_WARNING == APP_CHG_GetChargeStatus()) {
            st_event_warning_recovery_charging_data.type = E_CHARGING_WARNING_RECOVERY_CP_ABNORMAL;
            APP_CHG_EventPub(E_EVENT_CHG_WARNING_RECOVERY, st_event_warning_recovery_charging_data.type);
        }
        else if(E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()) {
            if(cp_state == E_CP_12V_DC) //保护暂停中有拔枪动作
            {
                APP_CHG_EventPubTopic(E_EVENT_CHG_PLUG_OUT);
            } else {
                st_event_protection_recovery_charging_data.type = E_CHARGING_PROTECTION_RECOVERY_CP_ABNORMAL; //保护暂停->保护恢复
                APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_RECOVERY, st_event_protection_recovery_charging_data.type);
            }
        }
        break;

    default:
        break;
    }
}

/**
 * @brief 核心CP异常电压滤波处理函数
 */
static uint32_t CP_Voltage_Filter(void) {

    uint32_t filtered_voltage;
    static bool is_in_abnormal_state = false; // 新增: 记录是否处于持续异常状态

    filtered_voltage = RTE_Call_AdcM_Vlotage(ADCMCFG_CP_CH);  //获取CP电压

    // 1. 检查是否处于忽略窗口期内 (通过计数器实现)
    if (g_cp_filter.ignore_counter > 0) {
        g_cp_filter.ignore_counter--; // 计数器减1
        // 在忽略窗口内, 直接返回上一个有效值
        return g_cp_filter.last_valid_voltage;
    }

    // 3. 检查滤波后的电压是否处于异常区域
    if (is_in_abnormal_zone(filtered_voltage)) {
        // 电压处于异常区域
        if (!is_in_abnormal_state) {
            // 首次检测到异常: 启动忽略窗口, 返回上次有效值
            is_in_abnormal_state = true;
            g_cp_filter.ignore_counter = 6; // 忽略接下来2次数据 (10ms)
            return g_cp_filter.last_valid_voltage;
        } else {
            // 已经处于持续异常状态: 直接返回当前异常值
            g_cp_filter.last_valid_voltage = filtered_voltage; // 更新为当前异常值
            return filtered_voltage;
        }
    } else {
        // 4. 电压正常区域
        is_in_abnormal_state = false; // 重置异常状态标志

        // 更新最后一次的有效值并返回
        g_cp_filter.last_valid_voltage = filtered_voltage;
        return filtered_voltage;
    }
}

/**
 * @brief CP 运行任务5ms周期
 */
void APP_CP_Task(void)
{
	#if (PRODUCT_TYPE == TYPE_3_5KW)
		APP_Adapter_detection(&st_Adapter_state_machine_machine);
	#endif
	
    uint32_t current_voltage = CP_Voltage_Filter();
    uint8_t pwm_active = APP_CP_GetPwmStatus(); //检测PWM=0、=1    =0 Pwm占空比100%  =1 Pwm占空比13%
    cp_voltage_state_t cp_state = APP_CP_DetectCPState(current_voltage, pwm_active); //CP状态检测, 检测当前CP电压值和active, 返回应设置波形
    cp_event_t event = APP_CP_ProcessStateMachine(&st_cp_state_machine, cp_state);

    if (event != E_EVENT_NONE)
    {
        APP_CP_HandleChargerEvent(event, cp_state);
    }
}

/**
 * @brief 获取当前CP状态
 */
cp_state_t APP_CP_GetCurrentStatus(void)
{
    return st_cp_state_machine.current_state;
}








