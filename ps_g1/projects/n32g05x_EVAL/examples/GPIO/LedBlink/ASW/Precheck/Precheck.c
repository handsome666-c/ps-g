#include "Precheck.h"

PreCheck_Manager_t Pre_Manager;

/*************************充电预检*************************/
// 充电初始化自检管理器
void PreCheck_Charging_Init(PreCheck_Manager_t *PreCheckManager) {
    PreCheckManager->charging.state = PRECHECK_IDLE;
    PreCheckManager->charging.timer = 0;
    PreCheckManager->charging.error.errorcode = PRECHECK_ERROR_CHARGING_NONE;
    PreCheckManager->charging.index.currentTestIndex = PRECHECK_INDEX_CHARGING_LEAKAGE;
    PreCheckManager->charging.subState = PRECHECK_SUBSTATE_IDLE;

    for (int i = 0; i < PRECHECK_INDEX_CHARGING_TEST_MAX; i++) {
        PreCheckManager->charging.testResults[i] = false;
        PreCheckManager->charging.faultStatus[i] = false;
    }
}

// 充电开始预检
void PreCheck_Charging_Start(PreCheck_Manager_t *PreCheckManager) {
    if (PreCheckManager->charging.state == PRECHECK_IDLE) {
        PreCheckManager->charging.state = PRECHECK_RUNNING;
        PreCheckManager->charging.timer = 0;
        PreCheckManager->charging.index.currentTestIndex = PRECHECK_INDEX_CHARGING_LEAKAGE;
        PreCheckManager->charging.error.errorcode = PRECHECK_ERROR_CHARGING_NONE;
        PreCheckManager->charging.subState = PRECHECK_SUBSTATE_START;
    }
}

// 充电切换到下一个测试项目
static void PreCheck_Charging_Next_Test(PreCheck_Manager_t *PreCheckManager) {
    PreCheckManager->charging.index.currentTestIndex++;
    PreCheckManager->charging.subState = PRECHECK_SUBSTATE_START;
    PreCheckManager->charging.timer = 0;

    if (PreCheckManager->charging.index.currentTestIndex >= PRECHECK_INDEX_CHARGING_TEST_MAX) {
        PreCheckManager->charging.index.currentTestIndex = PRECHECK_INDEX_CHARGING_TEST_MAX; // 进入完成状态
    }
}

static uint8_t PreCheck_Charging_FaultMap(uint8_t errorCode)
{
    uint8_t mapErrorCode = 0xFF;

    switch(errorCode)
    {
        case 0:
            mapErrorCode = 0;
            break;
        default:
            mapErrorCode = 0xFF;
            break;
    }
    return mapErrorCode;
}

// 完成预检
static void PreCheck_Charging_Complete(PreCheck_Manager_t *PreCheckManager) {
    bool allPassed = true;
    uint8_t errorCode = 0xFF;
    event_fault_charging_data_t st_event_fault_charging_data;

    for (int i = 0; i < PRECHECK_INDEX_CHARGING_TEST_MAX; i++) {
        if (PreCheckManager->charging.testResults[i]) {
            allPassed = false;
            errorCode = i;
            break;
        }
    }

    if (allPassed) {
        rt_kprintf("PreCheckManager->charging.state = PRECHECK_PASSED\r\n");
        PreCheckManager->charging.state = PRECHECK_PASSED;
        APP_CHG_EventPubTopic(E_EVENT_CHG_PRECHECK_SUCCESS);
    } else {
        rt_kprintf("PreCheckManager->charging.state = PRECHECK_FAILED\r\n");
        PreCheckManager->charging.state = PRECHECK_FAILED;
        st_event_fault_charging_data.type = (fault_charging_type_t)PreCheck_Charging_FaultMap(errorCode);
        APP_CHG_EventPub(E_EVENT_CHG_PRECHECK_FAILURE, st_event_fault_charging_data.type);
    }
    PreCheckManager->charging.state = PRECHECK_IDLE;
}

// 预检状态机更新（在5ms定时器中断中调用）
void PreCheck_Charging_Update(PreCheck_Manager_t *PreCheckManager) {
    if (PreCheckManager->charging.state != PRECHECK_RUNNING) {
        return;
    }
    PreCheckManager->charging.timer += 5;

    switch (PreCheckManager->charging.index.currentTestIndex)
    {
        case PRECHECK_INDEX_CHARGING_LEAKAGE: // 漏电检测
        //  RTE_Call_PortIf_Clear_S_out_state();
        //  PreCheck_Charging_Next_Test(&Pre_Manager);
            switch(PreCheckManager->charging.subState) {
                case PRECHECK_SUBSTATE_START://漏电传感器先校准
                    RTE_Call_PortIf_Clear_S_out_state();      //清除漏电中断标志位
                    RTE_Call_DioIf_WriteChannel(HW_DIO_CH_TEST_IN, DIOIF_PIN_LOW); //控制IO输出低电平校准
                    if(PreCheckManager->charging.timer >= 100)
                    {
                        RTE_Call_DioIf_WriteChannel(HW_DIO_CH_TEST_IN, DIOIF_PIN_HIGH); //控制IO输出高电平结束
                        PreCheckManager->charging.subState = PRECHECK_SUBSTATE_WAITING;
                        PreCheckManager->charging.timer = 0;
                        PreCheckManager->charging.S_out_count = 0;
                        PreCheckManager->charging.faultStatus[PRECHECK_INDEX_CHARGING_LEAKAGE] = false;
                    }
                    break;
                case PRECHECK_SUBSTATE_WAITING:
                    PreCheckManager->charging.faultStatus[PRECHECK_INDEX_CHARGING_LEAKAGE] = RTE_Call_PortIf_Get_S_out_state(); //获取外部上升沿触发标志
                    if(!PreCheckManager->charging.faultStatus[PRECHECK_INDEX_CHARGING_LEAKAGE])
                    {
                        if(PreCheckManager->charging.timer > LEAKAGE_TIMEOUT_MS)
                        {
                            PreCheckManager->charging.testResults[PRECHECK_INDEX_CHARGING_LEAKAGE] = true; //true为故障
                            PreCheckManager->charging.error.errorcode = PRECHECK_ERROR_CHARGING_LEAKAGE;
                            PreCheckManager->charging.index.currentTestIndex = PRECHECK_INDEX_CHARGING_TEST_MAX;
                            RTE_Call_PortIf_Clear_S_out_state();
                            PreCheck_Charging_Next_Test(&Pre_Manager);
                        }
                    }
                    else
                    {
                        PreCheckManager->charging.subState = PRECHECK_SUBSTATE_CHECKING;
                        PreCheckManager->charging.timer = 0;
                        PreCheckManager->charging.S_out_count = 0;
                        PreCheckManager->charging.faultStatus[PRECHECK_INDEX_CHARGING_LEAKAGE] = false;
                    }
                    break;
                case PRECHECK_SUBSTATE_CHECKING:
                    if(PreCheckManager->charging.timer <= LEAKAGE_HIGH_LEVEL_DETECT_MS)
                    {
                        if(PreCheckManager->charging.timer % 100 == 0)
                        {
                            if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT) == DIOIF_PIN_HIGH)
                            {
                                PreCheckManager->charging.S_out_count = 0;
                            }
                            else
                            {
                                PreCheckManager->charging.S_out_count += 1;
                                if (PreCheckManager->charging.S_out_count > 3) //低电平连续超过10次，记录故障并执行下一命令检测
                                {
                                    PreCheckManager->charging.testResults[PRECHECK_INDEX_CHARGING_LEAKAGE] = true; //true为故障
                                    PreCheckManager->charging.error.errorcode = PRECHECK_ERROR_CHARGING_LEAKAGE;
                                    PreCheckManager->charging.index.currentTestIndex = PRECHECK_INDEX_CHARGING_TEST_MAX;
                                    RTE_Call_PortIf_Clear_S_out_state();    //底层漏电标志位
                                    PreCheck_Charging_Next_Test(&Pre_Manager);
                                }
                            }
                        }
                    }
                    else if((PreCheckManager->charging.timer > LEAKAGE_HIGH_LEVEL_DETECT_MS) && (PreCheckManager->charging.timer <= LEAKAGE_LOW_LEVEL_DETECT_MS))
                    {
                        if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT) == DIOIF_PIN_LOW) //1400ms以后检测高电平异常
                        {
                            PreCheckManager->charging.S_out_count = 0;
                        }
                        else
                        {
                            PreCheckManager->charging.S_out_count += 1;
                            if (PreCheckManager->charging.S_out_count > 10) //高电平连续超过10次记录故障并执行下一命令检测
                            {
                                PreCheckManager->charging.testResults[PRECHECK_INDEX_CHARGING_LEAKAGE] = true; //true为故障
                                PreCheckManager->charging.error.errorcode = PRECHECK_ERROR_CHARGING_LEAKAGE;
                                PreCheckManager->charging.index.currentTestIndex = PRECHECK_INDEX_CHARGING_TEST_MAX;
                                RTE_Call_PortIf_Clear_S_out_state();
                                PreCheck_Charging_Next_Test(&Pre_Manager);
                            }
                        }
                    }
                    else
                    {
                        RTE_Call_PortIf_Clear_S_out_state();
                        PreCheck_Charging_Next_Test(&Pre_Manager);
                    }
                    break;
                default:
                    PreCheckManager->charging.subState = PRECHECK_SUBSTATE_START;
                    break;
            }
            break;
        case PRECHECK_INDEX_CHARGING_TEST_MAX: // 自检完成
            PreCheck_Charging_Complete(&Pre_Manager);
            break;
    }
}

/*************************放电预检*************************/
// 充电初始化自检管理器
void PreCheck_Discharging_Init(PreCheck_Manager_t *PreCheckManager) {
    PreCheckManager->discharging.state = PRECHECK_IDLE;
    PreCheckManager->discharging.timer = 0;
    PreCheckManager->discharging.error.errorcode = PRECHECK_ERROR_DISCHARING_NONE;
    PreCheckManager->discharging.index.currentTestIndex = PRECHECK_INDEX_DISCHARING_RELAY_STICK;
    PreCheckManager->discharging.subState = PRECHECK_SUBSTATE_IDLE;

    for (int i = 0; i < PRECHECK_INDEX_CHARGING_TEST_MAX; i++) {
        PreCheckManager->discharging.testResults[i] = false;
        PreCheckManager->discharging.faultStatus[i] = false;
    }
}

// 充电开始预检
void PreCheck_Discharging_Start(PreCheck_Manager_t *PreCheckManager) {
    if (PreCheckManager->discharging.state == PRECHECK_IDLE) {
        PreCheckManager->discharging.state = PRECHECK_RUNNING;
        PreCheckManager->discharging.timer = 0;
        PreCheckManager->discharging.index.currentTestIndex = PRECHECK_INDEX_DISCHARING_RELAY_STICK;
        PreCheckManager->discharging.error.errorcode = PRECHECK_ERROR_DISCHARING_NONE;
        PreCheckManager->discharging.subState = PRECHECK_SUBSTATE_START;
    }
}

// 充电切换到下一个测试项目
static void PreCheck_Discharging_Next_Test(PreCheck_Manager_t *PreCheckManager) {
    PreCheckManager->discharging.index.currentTestIndex++;
    PreCheckManager->discharging.subState = PRECHECK_SUBSTATE_START;
    PreCheckManager->discharging.timer = 0;

    if (PreCheckManager->discharging.index.currentTestIndex >= PRECHECK_INDEX_DISCHARING_TEST_MAX) {
        PreCheckManager->discharging.index.currentTestIndex = PRECHECK_INDEX_DISCHARING_TEST_MAX; // 进入完成状态
    }
}

static uint8_t PreCheck_Discharging_FaultMap(uint8_t errorCode)
{
    uint8_t mapErrorCode = 0xFF;

    switch(errorCode)
    {
        case 0:
            mapErrorCode = 2;
            break;
        default:
            mapErrorCode = 0xFF;
            break;
    }
    return mapErrorCode;
}

// 完成预检
static void PreCheck_Discharging_Complete(PreCheck_Manager_t *PreCheckManager) {
    bool allPassed = true;
    static bool historyallPassed = true;
    uint8_t errorCode = 0xFF;
    event_warning_discharging_data_t st_event_warning_discharging_data;
    event_warning_recovery_discharging_data_t st_event_warning_recovery_discharging_data;

    for (int i = 0; i < PRECHECK_INDEX_CHARGING_TEST_MAX; i++) {
        if (PreCheckManager->discharging.testResults[i]) {
            allPassed = false;
            historyallPassed = false;
            errorCode = i;
            break;
        }
    }
    if (allPassed) {
        if(historyallPassed)
        {
            PreCheckManager->discharging.state = PRECHECK_PASSED;
            APP_DISCHG_EventPubTopic(E_EVENT_DISCHG_PRECHECK_SUCCESS);
        }
        else
        {
            PreCheckManager->discharging.state = PRECHECK_PASSED;
            st_event_warning_recovery_discharging_data.type = E_DISCHARGING_WARNING_RECOVERY_RELAY_STICK;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_WARNING_RECOVERY, st_event_warning_recovery_discharging_data.type);
        }
    } else {
        PreCheckManager->discharging.state = PRECHECK_FAILED;
        st_event_warning_discharging_data.type = (warning_discharging_type_t)PreCheck_Discharging_FaultMap(errorCode);
        APP_DISCHG_EventPub(E_EVENT_DISCHG_PRECHECK_FAILURE, st_event_warning_discharging_data.type);
    }
    if(PreCheckManager->discharging.state == PRECHECK_PASSED)
    {
        PreCheckManager->discharging.state = PRECHECK_IDLE;
    }
    else if(PreCheckManager->discharging.state == PRECHECK_FAILED)
    {
        PreCheckManager->discharging.timer = 0;
        PreCheckManager->discharging.subState = PRECHECK_SUBSTATE_WAITING;
        PreCheckManager->discharging.index.currentTestIndex = PRECHECK_INDEX_DISCHARING_RELAY_STICK;
    }
}

// 自检状态机更新（在5ms定时器中断中调用）
void PreCheck_Discharging_Update(PreCheck_Manager_t *PreCheckManager) {
    if ((PreCheckManager->discharging.state != PRECHECK_RUNNING) && (PreCheckManager->discharging.state != PRECHECK_FAILED)) {
        return;
    }
    PreCheckManager->discharging.timer += 5;

    switch (PreCheckManager->discharging.index.currentTestIndex)
    {
        case PRECHECK_INDEX_DISCHARING_RELAY_STICK: // 继电器粘连检测，继电器未闭合通过检测电压值判断是否粘连
            switch(PreCheckManager->discharging.subState)
            {
                case PRECHECK_SUBSTATE_START:
                    PreCheckManager->discharging.subState = PRECHECK_SUBSTATE_CHECKING;
                    PreCheckManager->discharging.timer = 0;
                    PreCheckManager->discharging.faultStatus[PRECHECK_INDEX_DISCHARING_RELAY_STICK] = false;
                    break;
                case PRECHECK_SUBSTATE_CHECKING:
                    // 周期性获取继电器粘连测函数
                    if(RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH) >= PRECHECK_DISCHG_RELAYCHK_THRESHOLD)//继电器未闭合，电压值超过10v认为继电器粘连
                    {
                        PreCheckManager->discharging.error.errorcode = PRECHECK_ERROR_DISCHARING_RELAY_STICK;
                        PreCheckManager->discharging.testResults[PRECHECK_INDEX_DISCHARING_RELAY_STICK] = true;//true为故障
                        PreCheckManager->discharging.index.currentTestIndex = PRECHECK_INDEX_DISCHARING_TEST_MAX;
                        PreCheck_Discharging_Next_Test(&Pre_Manager);
                    }
                    break;
                case PRECHECK_SUBSTATE_WAITING:
                    // 周期性获取继电器粘连测函数
                    if(RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH) < PRECHECK_DISCHG_RELAYCHK_THRESHOLD)//继电器未闭合，电压值超过10v认为继电器粘连
                    {
                        PreCheckManager->discharging.error.errorcode = PRECHECK_ERROR_DISCHARING_NONE;
                        PreCheckManager->discharging.testResults[PRECHECK_INDEX_DISCHARING_RELAY_STICK] = false;//true为故障
                        PreCheckManager->discharging.index.currentTestIndex = PRECHECK_INDEX_DISCHARING_TEST_MAX;
                        PreCheck_Discharging_Next_Test(&Pre_Manager);
                    }
                    break;
                default:
                    PreCheckManager->discharging.subState = PRECHECK_SUBSTATE_START;
                    break;
            }
            break;
        case PRECHECK_INDEX_DISCHARING_TEST_MAX: // 自检完成
            PreCheck_Discharging_Complete(&Pre_Manager);
            break;
    }
}
