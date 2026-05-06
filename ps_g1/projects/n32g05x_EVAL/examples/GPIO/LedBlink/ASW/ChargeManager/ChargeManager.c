#include "ChargeManager.h"

ChargingStateMachine machine;
uint8_t DischgorchgSysState = SYSSTATE_CHARGING;

/**
 *\name    chargingStateMachineInit.
 *\fun     初始化充电状态机
 *\param   ChargingStateMachine *machine
 *\return  none
 */
void chargingStateMachineInit(ChargingStateMachine *machine)
{
    machine->currentState = CHARGING_STATE_IDLE;
    machine->stateTimer = 0;
    machine->powerOnStableTime = CHARGING_POWER_ON_STABLE_TIME_MS;
    machine->selfCheckTime = SELF_CHECK_TIME_MS;
    machine->selfCheckResult = false;
}

/**
 *\name    startChargingProcess.
 *\fun     开始充电流程
 *\param   ChargingStateMachine *machine
 *\return  none
 */
static void startChargingProcess(ChargingStateMachine *machine)
{
    if (machine->currentState == CHARGING_STATE_IDLE)
    {
        machine->powerOnStableTime = CHARGING_POWER_ON_STABLE_TIME_MS;
        machine->currentState = CHARGING_STATE_POWER_ON_WAIT;
        machine->stateTimer = 0;
    }
}

/**
 *\name    startDischargingProcess.
 *\fun     开始放电流程
 *\param   ChargingStateMachine *machine
 *\return  none
 */
static void startDischargingProcess(ChargingStateMachine *machine)
{
    if (machine->currentState == CHARGING_STATE_IDLE)
    {
        machine->powerOnStableTime = DISCHARGING_POWER_ON_STABLE_TIME_MS;
        machine->currentState = DISCHARGING_STATE_POWER_ON_WAIT;
        machine->stateTimer = 0;
    }
}

/**
 *\name    startChargingRunning.
 *\fun     开始充电主功能流程
 *\param   ChargingStateMachine *machine
 *\return  none
 */
static void startChargingRunning(ChargingStateMachine *machine)
{
    if (machine->currentState == CHARGING_STATE_SELF_CHECK)
    {
        machine->currentState = CHARGING_STATE_RUNNING;
        machine->stateTimer = 0;
    }
}

/**
 *\name    startDishargingRunning.
 *\fun     开始放电主功能流程
 *\param   ChargingStateMachine *machine
 *\return  none
 */
static void startDishargingRunning(ChargingStateMachine *machine)
{
    if(machine->currentState == DISCHARGING_STATE_SELF_CHECK)
    {
        machine->currentState = DISCHARGING_STATE_RUNNING;
        machine->stateTimer = 0;
    }
}

/**
 *\name    Enter_RunningChargingMode.
 *\fun     使能充电主功能
 *\param   none
 *\return  none
 */
void Enter_RunningChargingMode(void)
{
    startChargingRunning(&machine);
}

/**
 *\name    Enter_RunningDischargingMode.
 *\fun     使能放电主功能
 *\param   none
 *\return  none
 */
void Enter_RunningDischargingMode(void)
{
    startDishargingRunning(&machine);
}

/**
 *\name    chargingStateMachine_setcalibration.
 *\fun     状态机状态设置校准模式
 *\param   ChargingStateMachine *machine
 *\return  none
 */
void chargingStateMachine_setcalibration(ChargingStateMachine *machine)
{
    machine->currentState = CALIBRATION_MODE;
}

/**
 *\name    chargingStateMachine_setfactory.
 *\fun     状态机状态设置工厂模式
 *\param   ChargingStateMachine *machine
 *\return  none
 */
void chargingStateMachine_setfactory(ChargingStateMachine *machine)
{
    machine->currentState = FACTORY_MODE;
}

/**
 *\name    chargingStateMachineHandler.
 *\fun     充放电状态机处理函数，周期：5ms
 *\param   ChargingStateMachine *machine, uint32_t elapsedMs
 *\return  none
 */
void chargingStateMachineHandler(ChargingStateMachine *machine, uint32_t elapsedMs)
{
    machine->stateTimer += elapsedMs; // 更新状态计时器
    switch (machine->currentState)
    {
    case CHARGING_STATE_IDLE:  // 空闲状态，等待启动命令
        if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_AC_DT) == DIOIF_PIN_HIGH)  //充电模式    //AC_DT高电平进入充电逻辑，反之进入放电逻辑
        {
            startChargingProcess(machine);
        }
        else //放电模式
        {
            startDischargingProcess(machine);
        }
//        rt_kprintf("PS_G1 power on!!!\r\n");
        break;

    case CHARGING_STATE_POWER_ON_WAIT:  // 充电上电等待系统稳定状态
        if (machine->stateTimer >= machine->powerOnStableTime)
        {
            machine->currentState = CHARGING_STATE_INIT;
            machine->stateTimer = 0;
        }
        break;

    case DISCHARGING_STATE_POWER_ON_WAIT:  // 放电上电等待系统稳定状态
        if (machine->stateTimer >= machine->powerOnStableTime)
        {
            machine->currentState = DISCHARGING_STATE_INIT;
            machine->stateTimer = 0;
        }
        break;

    case CHARGING_STATE_INIT:
        RTE_Call_DioIf_WriteChannel(HW_DIO_CH_CP_CONT,DIOIF_PIN_HIGH); //控制IO输出高电平导通CP
        RTE_Call_PwmIf_Set_Duty(TIM5, OC3, 1000); //CP输出高电平
        if (machine->stateTimer < 1000)
        {
            RTE_Call_PwmIf_Set_Duty(TIM2, OC2, 100);//小，1s-100%再60%
        } else {
            DischgorchgSysState = SYSSTATE_CHARGING;
            RTE_Call_PwmIf_Set_Duty(TIM2, OC2, 60);
            SelfTest_Charging_Init(&Self_Manager);
            SelfTest_Charging_Start(&Self_Manager);
            PreCheck_Charging_Init(&Pre_Manager);//预检初始化
            APP_DiodeCheck_Init();//二极管检测初始化
            APP_CHG_StateMachineInit();
            APP_DIAG_CHG_Init();
            machine->currentState = CHARGING_STATE_SELF_CHECK;
            machine->stateTimer = 0;
        }
        break;

    case DISCHARGING_STATE_INIT:
        RTE_Call_DioIf_WriteChannel(HW_DIO_CH_CP_CONT,DIOIF_PIN_LOW); //控制IO输出低电平断开CP
        RTE_Call_PwmIf_Set_Duty(TIM2, OC2, 0);//直接关闭小继电器
//        RTE_Call_PwmIf_Set_Duty(TIM5, OC3, 0);  //CP输出低电平
        DischgorchgSysState = SYSSTATE_DISCHARGING;
        SelfTest_Discharging_Init(&Self_Manager);
        SelfTest_Discharging_Start(&Self_Manager);
        PreCheck_Discharging_Init(&Pre_Manager);
        APP_DISCHG_StateMachineInit();
        APP_DIAG_DISCHG_Init();
        machine->currentState = DISCHARGING_STATE_SELF_CHECK;
        machine->stateTimer = 0;
        break;

    case CHARGING_STATE_SELF_CHECK: // 自检状态-充电
        APP_CP_Task();
        SelfTest_Charging_Update(&Self_Manager);//继电器粘连检测、漏电检测、短路检测、过压检测、欠压检测
        break;

    case DISCHARGING_STATE_SELF_CHECK:  // 自检状态-放电
        SelfTest_Discharging_Update(&Self_Manager);//漏电检测
        break;

    case CHARGING_STATE_RUNNING:  // 运行功能状态
        APP_CP_Task();//CP状态判断
        APP_DIAG_CHG_Update();  //故障诊断
        PreCheck_Charging_Update(&Pre_Manager);//预检
        APP_DiodeCheck_Update(); //二极管检测
        APP_CHG_StateMachineDispatch();
        APP_RelayTask();//继电器任务
        break;

    case DISCHARGING_STATE_RUNNING:
        APP_DIAG_DISCHG_Update();
        PreCheck_Discharging_Update(&Pre_Manager);
        APP_DISCHG_StateMachineDispatch();
        APP_RelayTask();//继电器任务
        break;

    case CHARGING_STATE_FAULT:  // 充电故障状态
        break;
    case DISCHARGING_STATE_FAULT:  // 放点故障状态
        break;

    case CALIBRATION_MODE:  // 校准模式
        Calibration_Update(&calibration_Manager); //校准状态机
        APP_RelayTask();//继电器任务
        break;

    case FACTORY_MODE:  // 工厂模式
        break;
    }
}

/**
 *\name    getSysState.
 *\fun     获取当前充放电状态
 *\param   none
 *\return  DischgorchgSysState
 */
uint8_t getSysState(void)
{
    return DischgorchgSysState;
}

/**
 *\name    APP_ChargeManagerInit.
 *\fun     充电状态机管理初始化
 *\param   none
 *\return  none
 */
void APP_ChargeManagerInit(void)
{
    chargingStateMachineInit(&machine);
}

/**
 *\name    APP_ChargeManageMain.
 *\fun     充电状态机管理主函数
 *\param   none
 *\return  none
 */
void APP_ChargeManageMain(void)
{
    chargingStateMachineHandler(&machine, 5);
}
