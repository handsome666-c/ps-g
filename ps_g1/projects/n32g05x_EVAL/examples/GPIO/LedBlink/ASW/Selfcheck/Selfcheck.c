#include "Selfcheck.h"

SelfTest_Manager_t Self_Manager;

/***************************充电自检***************************/
// 充电-初始化自检管理器
void SelfTest_Charging_Init(SelfTest_Manager_t *selfTestManager) {
    selfTestManager->charging.state = SELF_TEST_IDLE;
    selfTestManager->charging.timer = 0;
    selfTestManager->charging.S_out_count = 0;
    selfTestManager->charging.error.errorcode = ERROR_CHARGING_NONE;
    selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_RELAY_STICK;
    selfTestManager->charging.subState = SUBSTATE_IDLE;
    for (int i = 0; i < Selfcheck_INDEX_CHARGING_RELAY_STICK; i++) {
        selfTestManager->charging.testResults[i] = false;
        selfTestManager->charging.faultStatus[i] = false;
    }
}

// 充电-开始自检
void SelfTest_Charging_Start(SelfTest_Manager_t *selfTestManager) {
    if (selfTestManager->charging.state == SELF_TEST_IDLE) {
        selfTestManager->charging.state = SELF_TEST_RUNNING;
        selfTestManager->charging.timer = 0;
        selfTestManager->charging.S_out_count = 0;
        selfTestManager->charging.error.errorcode = ERROR_CHARGING_NONE;
        selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_RELAY_STICK;
        selfTestManager->charging.subState = SUBSTATE_START;
    }
}

// 充电-切换到下一个测试项目
static void SelfTest_Charging_Next_Test(SelfTest_Manager_t *selfTestManager) {
    selfTestManager->charging.index.currentTestIndex++;
    selfTestManager->charging.subState = SUBSTATE_START;
    selfTestManager->charging.timer = 0;
    if (selfTestManager->charging.index.currentTestIndex >= Selfcheck_INDEX_CHARGING_TEST_MAX) {
        selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX; // 进入完成状态
    }
}

/**
 * @brief 上电自检 - 检查充电枪连接状态
 * @return true: 枪已连接, false: 枪未连接, 后续调整至CP文件内
 */
bool POST_CheckGunInsertion(void) {
    float cp_value = RTE_Call_AdcM_Vlotage(ADCMCFG_CP_CH);
    bool gun_detected = false;
    cp_voltage_state_t cp_state = APP_CP_DetectCPState(cp_value, 0);
    if (cp_state == E_CP_12V_DC) {
        gun_detected = false;
    } else {
        gun_detected = true;
    }
    return gun_detected;
}

/*短路检测IO */
static bool SelfTest_Charging_ShortCircuit_Check(SelfTest_Manager_t *selfTestManager) {
    // 1. 读取硬件IO
    bool currentFaultStatus = false;
    static uint8_t debounceCounter = 0;
    static uint16_t timer = 0;
    timer += 5;
    RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT, DIOIF_PIN_HIGH); // 闭合短路检测IO
    if (timer > 100) {
        if (RTE_Call_AdcM_Vlotage(ADCMCFG_Shortcircuit_CH) == 1) {
            currentFaultStatus = true;
        }
    }

    // 2. 实现消抖逻辑
    if (currentFaultStatus) {
        if (debounceCounter < 10) {
            debounceCounter++;
        }
    } else {
        debounceCounter = 0; // 一旦发现没问题, 计数器清零
    }

    // 3. 更新最终状态
    return selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_SHORTCIRCUIT] = (debounceCounter >= 10);
}

bool ShortCircuit_GettestResults(SelfTest_Manager_t *selfTestManager) {
    return selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_SHORTCIRCUIT];
}

static uint8_t SelfTest_Charging_FaultMap(uint8_t errorCode) //映射到充电部分故障, 充电与自检测检测顺序不同
{
    uint8_t mapErrorCode = 0xFF;
    switch (errorCode) {
//        case 0: //PGD故障
//            mapErrorCode = 8;
//            break;
        case 0: //继电器粘
            mapErrorCode = 2;
            break;
        case 1: //漏电
            mapErrorCode = 0;
            break;
        case 2: //短路
            mapErrorCode = 1;
            break;
        case 3: //过压
            mapErrorCode = 4;
            break;
        case 4: //欠压
            mapErrorCode = 4;
            break;
        default: //其他
            mapErrorCode = 0xFF;
            break;
    }
    return mapErrorCode;
}

// 完成自检
static void SelfTest_Charging_Complete(SelfTest_Manager_t *selfTestManager) {
    bool allPassed = true;
    uint8_t errorCode = 0xFF;
    event_fault_charging_data_t st_event_fault_charging_data;

    for (int i = 0; i < Selfcheck_INDEX_CHARGING_TEST_MAX; i++) {
        if (selfTestManager->charging.testResults[i] == true) {
            allPassed = false;
            errorCode = i;
            break;
        }
    }

    if (allPassed == true) {
        selfTestManager->charging.state = SELF_TEST_PASSED;
        rt_kprintf("selfTestManager->charging.state = SELF_TEST_PASSED;\r\n");
        if (E_STATE_UNPLUGGED == APP_CP_GetCurrentStatus()) //12dc 12pwm
        {
            APP_CHG_EventPubTopic(E_EVENT_CHG_POST_SUCCESS_IDLE); // 自检完成进入空闲状态
        } else if (E_STATE_PLUGGED == APP_CP_GetCurrentStatus()) //9dc 9VPWM
        {
            APP_CHG_EventPubTopic(E_EVENT_CHG_POST_SUCCESS_CONNECTED); // 自检完成进入插枪状态
        } else if (E_STATE_EV_READY == APP_CP_GetCurrentStatus()) //6dc 6pwm
        {
            APP_CHG_EventPubTopic(E_EVENT_CHG_POST_SUCCESS_CONNECTED); // 自检完成进入插枪状态
        //未知状态-失败
        } else //未知状态-失败
        {
            st_event_fault_charging_data.type = E_CHARGING_FAULT_CP_ABNORMAL;
            APP_CHG_EventPub(E_EVENT_CHG_POST_FAILURE, st_event_fault_charging_data.type);
        }
    } else {
        selfTestManager->charging.state = SELF_TEST_FAILED;
        rt_kprintf("selfTestManager->charging.state = SELF_TEST_FAILED;\r\n");
        st_event_fault_charging_data.type = (fault_charging_type_t)SelfTest_Charging_FaultMap(errorCode);
        APP_CHG_EventPub(E_EVENT_CHG_POST_FAILURE, st_event_fault_charging_data.type);
    }
}

void SelfTest_Charging_Update(SelfTest_Manager_t *selfTestManager) {
    if (selfTestManager->charging.state != SELF_TEST_RUNNING) {
        return;
    }
    selfTestManager->charging.timer += 5;
    switch (selfTestManager->charging.index.currentTestIndex) {
        case Selfcheck_INDEX_CHARGING_RELAY_STICK: // 继电器黏连检测
            //上电非空闲状态下不进行继电器黏连检测
            if(true == POST_CheckGunInsertion())
            {
                SelfTest_Charging_Next_Test(&Self_Manager);
                break;
            }
            switch(selfTestManager->charging.subState) {
                case SUBSTATE_START:
                    selfTestManager->charging.subState = SUBSTATE_CHECKING;
                    selfTestManager->charging.timer = 0;
                    selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_RELAY_STICK] = false;
                    break;
                case SUBSTATE_CHECKING:
                    // 周期性获取接地保护检测函数, 实现实时监控和消抖
                    selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_RELAY_STICK] = RTE_Call_AdcM_Vlotage(ADCMCFG_RelayCHK_CH); // 调用RTE层获取PGD状态
                    if(selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_RELAY_STICK] != false)
                    {
                        selfTestManager->charging.error.errorcode = ERROR_CHARGING_RELAY_STICK_FAULT;
                        selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_RELAY_STICK] = true;//true为故障
                        selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                    }
                    SelfTest_Charging_Next_Test(&Self_Manager);
                    break;
                default:
                    selfTestManager->charging.subState = SUBSTATE_START;
                    break;
            }
            break;

        case Selfcheck_INDEX_CHARGING_LEAKAGE: // 漏电检测
            switch(selfTestManager->charging.subState) {
                case SUBSTATE_START://漏电传感器先校准
                    RTE_Call_PortIf_Clear_S_out_state();      //清除漏电中断标志位
                    RTE_Call_DioIf_WriteChannel(HW_DIO_CH_TEST_IN, DIOIF_PIN_LOW); // 控制IO输出低电平100ms校准
                    if(selfTestManager->charging.timer >= 100)
                    {
                        RTE_Call_DioIf_WriteChannel(HW_DIO_CH_TEST_IN, DIOIF_PIN_HIGH); // 控制IO输出高电平结束
                        selfTestManager->charging.subState = SUBSTATE_WAITING;
                        selfTestManager->charging.timer = 0;
                        selfTestManager->charging.S_out_count = 0;
                        selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_LEAKAGE] = false;
                    }
                    break;
                case SUBSTATE_WAITING:
                    selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_LEAKAGE] = RTE_Call_PortIf_Get_S_out_state(); // 获取外部上升沿触发标志
                    if(selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_LEAKAGE] == false)
                    {
                        if(selfTestManager->charging.timer > LEAKAGE_TIMEOUT_MS)
                        {
                            selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_LEAKAGE] = true; // true为故障
                            selfTestManager->charging.error.errorcode = ERROR_CHARGING_LEAKAGE_FAULT;
                            selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                            RTE_Call_PortIf_Clear_S_out_state();
                            SelfTest_Charging_Next_Test(&Self_Manager);
                        }
                    } else {
                        selfTestManager->charging.subState = SUBSTATE_CHECKING;
                        selfTestManager->charging.timer = 0;
                        selfTestManager->charging.S_out_count = 0;
                        selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_LEAKAGE] = false;
                    }
                    break;
                case SUBSTATE_CHECKING:
                    if(selfTestManager->charging.timer <= LEAKAGE_HIGH_LEVEL_DETECT_MS)
                    {
                        if(selfTestManager->charging.timer%100 == 0)
                        {
                            if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT) == DIOIF_PIN_HIGH)
                            {
                                selfTestManager->charging.S_out_count = 0;
                            } else {
                                selfTestManager->charging.S_out_count += 1;
                                if (selfTestManager->charging.S_out_count > 300) // 低电平连续超过300次, 记录故障并执行下一命令检测
                                {
                                    selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_LEAKAGE] = true; // true为故障
                                    selfTestManager->charging.error.errorcode = ERROR_CHARGING_LEAKAGE_FAULT;
                                    selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                                    RTE_Call_PortIf_Clear_S_out_state();    // 底层漏电标志位
                                    SelfTest_Charging_Next_Test(&Self_Manager);
                                }
                            }
                        }
                    } else if((selfTestManager->charging.timer100ms >= LEAKAGE_HIGH_LEVEL_DETECT_MS) && (selfTestManager->charging.timer <= LEAKAGE_LOW_LEVEL_DETECT_MS))
                    {
                        if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT) == DIOIF_PIN_LOW) // 检测高电平异常
                        {
                            selfTestManager->charging.S_out_count = 0;
                        } else {
                            selfTestManager->charging.S_out_count += 1;
                            if (selfTestManager->charging.S_out_count > 10) // 高电平连续超过10次记录故障并执行下一命令检测
                            {
                                selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_LEAKAGE] = true; // true为故障
                                selfTestManager->charging.error.errorcode = ERROR_CHARGING_LEAKAGE_FAULT;
                                selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                                RTE_Call_PortIf_Clear_S_out_state();
                                SelfTest_Charging_Next_Test(&Self_Manager);
                            }
                        }
                    } else {
                        RTE_Call_PortIf_Clear_S_out_state();
                        SelfTest_Charging_Next_Test(&Self_Manager);
                    }
                    break;
                default:
                    selfTestManager->charging.subState = SUBSTATE_START;
                    break;
            }
            break;

        case Selfcheck_INDEX_CHARGING_SHORTCIRCUIT: // 短路检测
            //上电插枪状态下不进行短路检测, 跳过短路自检
            if(true == POST_CheckGunInsertion())
            {
                SelfTest_Charging_Next_Test(&Self_Manager);
                break;
            }
            switch(selfTestManager->charging.subState) {
                case SUBSTATE_START:
                    selfTestManager->charging.subState = SUBSTATE_CHECKING;
                    selfTestManager->charging.timer = 0;
                    selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_SHORTCIRCUIT] = false;
                    Relay_hold_off(); // 短路自检, 先断开主回路继电器
                    break;
                case SUBSTATE_CHECKING:
                    Relay_hold_off(); // 短路自检, 先断开主回路继电器
                    SelfTest_Charging_ShortCircuit_Check(&Self_Manager);//短路检测
                    if (selfTestManager->charging.timer >= 200)// 100ms等待 100ms检测时间
                    {
                        // 最终检查经过消抖后的状态
                        RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT, DIOIF_PIN_LOW); // 断开短路检测IO
                        selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_SHORTCIRCUIT] = ShortCircuit_GettestResults(&Self_Manager);
                        if (selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_SHORTCIRCUIT] == true) // 低电平超过计数阈值, testResults = true
                        {
                            selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_SHORTCIRCUIT] = true; // true为故障
                            selfTestManager->charging.error.errorcode = ERROR_CHARGING_SHORTCIRCUIT_FAULT;
                            selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                        }
                        SelfTest_Charging_Next_Test(&Self_Manager);
                    }
                    break;
                default:
                    selfTestManager->charging.subState = SUBSTATE_START;
                    break;
            }
            break;

        case Selfcheck_INDEX_CHARGING_OVERVOLTAGE: // 过压检测
            switch(selfTestManager->charging.subState) {
                case SUBSTATE_START:
                    selfTestManager->charging.subState = SUBSTATE_CHECKING;
                    selfTestManager->charging.timer = 0;
                    selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_OVERVOLTAGE] = false;
                    break;
                case SUBSTATE_CHECKING:
                    if(RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH) >= SELFCHECK_CHG_OVER_VOLTAGE_THRESHOLD)// 调用RTE层获取PGD状态, 275V过压
                    {
                        selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_OVERVOLTAGE] = true;
                    }
                    if(selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_OVERVOLTAGE] == true)
                    {
                        selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_OVERVOLTAGE] = true;
                        selfTestManager->charging.error.errorcode = ERROR_CHARGING_OVERVOLTAGE;
                        selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                    }
                    SelfTest_Charging_Next_Test(&Self_Manager);
                    break;
                default:
                    selfTestManager->charging.subState = SUBSTATE_START;
                    break;
            }
            break;

        case Selfcheck_INDEX_CHARGING_UNDERVOLTAGE: // 欠压检测
            switch(selfTestManager->charging.subState) {
                case SUBSTATE_START:
                    selfTestManager->charging.subState = SUBSTATE_CHECKING;
                    selfTestManager->charging.timer = 0;
                    selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_UNDERVOLTAGE] = false;
                    break;
                case SUBSTATE_CHECKING:
                    if(RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH) <= SELFCHECK_CHG_UNDER_VOLTAGE_THRESHOLD)// 调用RTE层获取PGD状态, 280V欠压
                    {
                        selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_UNDERVOLTAGE] = true;
                    }
                    if(selfTestManager->charging.faultStatus[Selfcheck_INDEX_CHARGING_UNDERVOLTAGE] == true)
                    {
                        selfTestManager->charging.testResults[Selfcheck_INDEX_CHARGING_UNDERVOLTAGE] = true;
                        selfTestManager->charging.error.errorcode = ERROR_CHARGING_UNDERVOLTAGE;
                        selfTestManager->charging.index.currentTestIndex = Selfcheck_INDEX_CHARGING_TEST_MAX;
                    }
                    SelfTest_Charging_Next_Test(&Self_Manager);
                    break;
                default:
                    selfTestManager->charging.subState = SUBSTATE_START;
                    break;
            }
            break;

        case Selfcheck_INDEX_CHARGING_TEST_MAX: // 自检完成
            SelfTest_Charging_Complete(&Self_Manager);
            Enter_RunningChargingMode();
            break;
    }
}

/***************************放电自检***************************/
void SelfTest_Discharging_Init(SelfTest_Manager_t *selfTestManager) {
    selfTestManager->discharging.state = SELF_TEST_IDLE;
    selfTestManager->discharging.timer = 0;
    selfTestManager->discharging.S_out_count = 0;
    selfTestManager->discharging.error.errorcode = ERROR_DISCHARGING_NONE;
    selfTestManager->discharging.index.currentTestIndex = Selfcheck_INDEX_DISCHARGING_LEAKAGE;
    selfTestManager->discharging.subState = SUBSTATE_IDLE;
    for (int i = 0; i < Selfcheck_INDEX_DISCHARGING_TEST_MAX; i++) {
        selfTestManager->discharging.testResults[i] = false;
        selfTestManager->discharging.faultStatus[i] = false;
    }
}

// 开始自检
void SelfTest_Discharging_Start(SelfTest_Manager_t *selfTestManager) {
    if (selfTestManager->discharging.state == SELF_TEST_IDLE) {
        selfTestManager->discharging.state = SELF_TEST_RUNNING;
        selfTestManager->discharging.timer = 0;
        selfTestManager->discharging.S_out_count = 0;
        selfTestManager->discharging.error.errorcode = ERROR_DISCHARGING_NONE;
        selfTestManager->discharging.index.currentTestIndex = Selfcheck_INDEX_DISCHARGING_LEAKAGE;
        selfTestManager->discharging.subState = SUBSTATE_START;
    }
}

// 切换到下一个测试项目
static void SelfTest_Discharging_Next_Test(SelfTest_Manager_t *selfTestManager) {
    selfTestManager->discharging.index.currentTestIndex++;
    selfTestManager->discharging.subState = SUBSTATE_START;
    selfTestManager->discharging.timer = 0;
    if (selfTestManager->discharging.index.currentTestIndex >= Selfcheck_INDEX_DISCHARGING_TEST_MAX) {
        selfTestManager->discharging.index.currentTestIndex = Selfcheck_INDEX_DISCHARGING_TEST_MAX; // 进入完成状态
    }
}

uint8_t SelfTest_Discharging_FaultMap(uint8_t errorCode) //映射到充电部分故障, 充电与自检测检测顺序不同
{
    uint8_t mapErrorCode = 0xFF;

    switch(errorCode)
    {
        case 0: //漏电
            mapErrorCode = 0;
            break;
        default: //其他
            mapErrorCode = 0xFF;
            break;
    }
    return mapErrorCode;
}

// 完成自检
static void SelfTest_Discharging_Complete(SelfTest_Manager_t *selfTestManager) {
    bool allPassed = true;
    uint8_t errorCode = 0xFF;
    event_fault_discharging_data_t st_event_fault_discharging_data;

    for (int i = 0; i < Selfcheck_INDEX_DISCHARGING_TEST_MAX; i++) {
        if (selfTestManager->discharging.testResults[i] == true) {
            allPassed = false;
            errorCode = i;
            break;
        }
    }

    if (allPassed == true) {
        selfTestManager->discharging.state = SELF_TEST_PASSED;
        rt_kprintf("selfTestManager->discharging.state = SELF_TEST_PASSED;\r\n");
        APP_DISCHG_EventPubTopic(E_EVENT_DISCHG_POST_SUCCESS);   // 自检完成进入空闲状态
    } else {
        selfTestManager->discharging.state = SELF_TEST_FAILED;
        rt_kprintf("selfTestManager->discharging.state = SELF_TEST_FAILED;\r\n");
        st_event_fault_discharging_data.type = (fault_discharging_type_t)SelfTest_Discharging_FaultMap(errorCode);
        APP_DISCHG_EventPub(E_EVENT_DISCHG_POST_FAILURE, st_event_fault_discharging_data.type);
    }
}

void SelfTest_Discharging_Update(SelfTest_Manager_t *selfTestManager)
{
    if (selfTestManager->discharging.state != SELF_TEST_RUNNING) {
        return;
    }

    selfTestManager->discharging.timer += 5;

    switch (selfTestManager->discharging.index.currentTestIndex)
    {
        case Selfcheck_INDEX_DISCHARGING_LEAKAGE: // 漏电检测
            switch(selfTestManager->discharging.subState)
            {
                case SUBSTATE_START://漏电传感器先校准
                    RTE_Call_PortIf_Clear_S_out_state();      //清除漏电中断标志位
                    RTE_Call_DioIf_WriteChannel(HW_DIO_CH_TEST_IN, DIOIF_PIN_LOW); //控制IO输出低电平校准
                    if(selfTestManager->discharging.timer >= 100)
                    {
                        RTE_Call_DioIf_WriteChannel(HW_DIO_CH_TEST_IN, DIOIF_PIN_HIGH); //控制IO输出高电平结束
                        selfTestManager->discharging.subState = SUBSTATE_WAITING;
                        selfTestManager->discharging.timer = 0;
                        selfTestManager->discharging.S_out_count = 0;
                        selfTestManager->discharging.faultStatus[Selfcheck_INDEX_DISCHARGING_LEAKAGE] = false;
                    }
                    break;
                case SUBSTATE_WAITING:
                    selfTestManager->discharging.faultStatus[Selfcheck_INDEX_DISCHARGING_LEAKAGE] = RTE_Call_PortIf_Get_S_out_state(); //获取外部上升沿触发标志
                    if(!selfTestManager->discharging.faultStatus[Selfcheck_INDEX_DISCHARGING_LEAKAGE])
                    {
                        if(selfTestManager->discharging.timer > LEAKAGE_TIMEOUT_MS)
                        {
                            selfTestManager->discharging.testResults[Selfcheck_INDEX_DISCHARGING_LEAKAGE] = true; //true为故障
                            selfTestManager->discharging.error.errorcode = ERROR_DISCHARGING_LEAKAGE_FAULT;
                            selfTestManager->discharging.index.currentTestIndex = Selfcheck_INDEX_DISCHARGING_TEST_MAX;
                            RTE_Call_PortIf_Clear_S_out_state();
                            SelfTest_Discharging_Next_Test(&Self_Manager);
                        }
                    }
                    else
                    {
                        selfTestManager->discharging.subState = SUBSTATE_CHECKING;
                        selfTestManager->discharging.timer = 0;
                        selfTestManager->discharging.S_out_count = 0;
                        selfTestManager->discharging.faultStatus[Selfcheck_INDEX_DISCHARGING_LEAKAGE] = false;
                    }
                    break;
                case SUBSTATE_CHECKING:
                    if(selfTestManager->discharging.timer <= LEAKAGE_HIGH_LEVEL_DETECT_MS)
                    {
                        if(selfTestManager->discharging.timer%100 == 0)
                        {
                            if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT) == DIOIF_PIN_HIGH)
                            {
                                selfTestManager->discharging.S_out_count = 0;
                            }
                            else
                            {
                                selfTestManager->discharging.S_out_count += 1;
                                if (selfTestManager->discharging.S_out_count > 3) //低电平连续超过10次，记录故障并执行下一命令检测
                                {
                                    selfTestManager->discharging.testResults[Selfcheck_INDEX_DISCHARGING_LEAKAGE] = true; //true为故障
                                    selfTestManager->discharging.error.errorcode = ERROR_DISCHARGING_LEAKAGE_FAULT;
                                    selfTestManager->discharging.index.currentTestIndex = Selfcheck_INDEX_DISCHARGING_TEST_MAX;
                                    RTE_Call_PortIf_Clear_S_out_state();    //底层漏电标志位
                                    SelfTest_Discharging_Next_Test(&Self_Manager);
                                }
                            }
                        }
                    }
                    else if((selfTestManager->discharging.timer > LEAKAGE_HIGH_LEVEL_DETECT_MS) && (selfTestManager->discharging.timer <= LEAKAGE_LOW_LEVEL_DETECT_MS))
                    {
                        if(RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT) == DIOIF_PIN_LOW) //1400ms以后检测高电平异常
                        {
                            selfTestManager->discharging.S_out_count = 0;
                        }
                        else
                        {
                            selfTestManager->discharging.S_out_count += 1;
                            if (selfTestManager->discharging.S_out_count > 10) //高电平连续超过10次记录故障并执行下一命令检测
                            {
                                selfTestManager->discharging.testResults[Selfcheck_INDEX_DISCHARGING_LEAKAGE] = true; //true为故障
                                selfTestManager->discharging.error.errorcode = ERROR_DISCHARGING_LEAKAGE_FAULT;
                                selfTestManager->discharging.index.currentTestIndex = Selfcheck_INDEX_DISCHARGING_TEST_MAX;
                                RTE_Call_PortIf_Clear_S_out_state();
                                SelfTest_Discharging_Next_Test(&Self_Manager);
                            }
                        }
                    }
                    else
                    {
                        RTE_Call_PortIf_Clear_S_out_state();
                        SelfTest_Discharging_Next_Test(&Self_Manager);
                    }
                    break;
                default:
                    selfTestManager->discharging.subState = SUBSTATE_START;
                    break;
            }
            break;
        case Selfcheck_INDEX_DISCHARGING_TEST_MAX: // 自检完成
            SelfTest_Discharging_Complete(&Self_Manager);
            Enter_RunningDischargingMode();
            break;
    }
}
