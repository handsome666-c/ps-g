#include "Diag.h"

Diagnostic_Manager_t Diagnostic_Manager;

/****************************充电诊断部分****************************/

/********漏电检测 1ms执行一次********/
void APP_DIAG_CHG_LeakageCheck_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.LeakageCheck.currentState = DIAGNOSTIC_CHG_LeakageCheck_IDLE;
    Diagnostic->charging.LeakageCheck.count = 0;
}

void APP_DIAG_CHG_LeakageCheck_Start(Diagnostic_Manager_t *Diagnostic)
{
    Diagnostic->charging.LeakageCheck.currentState = DIAGNOSTIC_CHG_LeakageCheck_START;
}

void APP_DIAG_CHG_LeakageCheck(Diagnostic_Manager_t *Diagnostic)
{
    event_fault_charging_data_t st_event_fault_charging_data;

    if(Diagnostic->charging.LeakageCheck.currentState != DIAGNOSTIC_CHG_LeakageCheck_START)
    {
        return;
    }

    if(E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus())
    {
    }
    else
    {
        APP_DIAG_CHG_LeakageCheck_Init(Diagnostic);
        return;
    }

    // 前置检查：如果系统有其他更高优先级的故障，且不是漏电故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_LEAKAGE)) {
        return; // 系统被其他故障锁定, 不处理
    }

    // 1. 读取硬件IO或ADC值
    if(ADCM_t.debug.leakage.flag)
    {
        Diagnostic->charging.LeakageCheck.Leakage_SCR_Status = ADCM_t.debug.leakage.leakagestatus;
    }
    else
    {
        Diagnostic->charging.LeakageCheck.Leakage_SCR_Status = RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT);
    }

    if(Diagnostic->charging.LeakageCheck.Leakage_SCR_Status == DIOIF_PIN_HIGH)
    {
        Diagnostic->charging.LeakageCheck.count += 1;
    }
    else
    {
        Diagnostic->charging.LeakageCheck.count = 0; // 一旦发现没问题, 计数器清零
    }

    // 3. 更新最终状态
    if(Diagnostic->charging.LeakageCheck.count >= 5)
    {
        st_event_fault_charging_data.type = E_CHARGING_FAULT_LEAKAGE;
        APP_CHG_EventPub(E_EVENT_CHG_FAULT, st_event_fault_charging_data.type);
    }
}

/********短路检测********/
void APP_DIAG_CHG_ShortCircuit_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.ShortCircuit.currentState = DIAGNOSTIC_CHG_SHORTCIRCUIT_IDLE;
    Diagnostic->charging.ShortCircuit.timer = 0;
    Diagnostic->charging.ShortCircuit.faultStatus = false;
}

// 开始自检
void APP_DIAG_ShortCircuit_Start(Diagnostic_Manager_t *Diagnostic) {
    if (Diagnostic->charging.ShortCircuit.currentState == DIAGNOSTIC_CHG_SHORTCIRCUIT_IDLE) {
        Diagnostic->charging.ShortCircuit.currentState = DIAGNOSTIC_CHG_SHORTCIRCUIT_START;
        Diagnostic->charging.ShortCircuit.timer = 0;
        Diagnostic->charging.ShortCircuit.faultStatus = false;
    }
}

void APP_DIAG_CHG_ShortCircuit_Trigger(Diagnostic_Manager_t *Diagnostic)
{
    static uint32_t timer5ms = 0;
    timer5ms += 1;
    if(timer5ms >= DIAG_CHG_SHORTCIRCUIT_TIMER)
    {
        if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus())
        {
            APP_DIAG_ShortCircuit_Start(&Diagnostic_Manager);
        }
        timer5ms = 0;
    }
}

void APP_DIAG_CHG_ShortCircuit_Check(Diagnostic_Manager_t *Diagnostic) {
    // 1. 读取硬件IO或ADC值
    bool currentFaultStatus = false;

    RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT,DIOIF_PIN_HIGH); //闭合短路检测IO
    if(RTE_Call_AdcM_Vlotage(ADCMCFG_Shortcircuit_CH) == 1)
    {
        currentFaultStatus = true;
    }

    // 2. 实现消抖逻辑
    static uint8_t debounceCounter = 0;

    if (currentFaultStatus) {
        if (debounceCounter < 10) {
            debounceCounter++;
        }
    } else {
        debounceCounter = 0; // 一旦发现没问题, 计数器清零
    }

    // 3. 更新最终状态
    Diagnostic->charging.ShortCircuit.faultStatus = (debounceCounter >= 10);
}

bool APP_DIAG_CHG_ShortCircuit_GetFaultStatus(Diagnostic_Manager_t *Diagnostic) {
    return (Diagnostic->charging.ShortCircuit.faultStatus);
}

void APP_DIAG_CHG_ShortCircuit_Update(Diagnostic_Manager_t *Diagnostic)
{
    event_fault_charging_data_t st_event_fault_charging_data;

    if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus()) //空闲中自检
    {
    }
    else
    {
        APP_DIAG_CHG_ShortCircuit_Init(&Diagnostic_Manager);
        return;
    }

    // 前置检查：如果系统有其他更高优先级的故障, 且不是短路故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_SHORT_CIRCUIT)) {
        return; // 系统被其他故障锁定, 不处理故障
    }

    if (DIAGNOSTIC_CHG_SHORTCIRCUIT_IDLE == Diagnostic->charging.ShortCircuit.currentState) {
        return;
    }

    Diagnostic->charging.ShortCircuit.timer += 5;

    switch (Diagnostic->charging.ShortCircuit.currentState)
    {
        case DIAGNOSTIC_CHG_SHORTCIRCUIT_START:
            Diagnostic->charging.ShortCircuit.currentState = DIAGNOSTIC_CHG_SHORTCIRCUIT_CHECKING;
            Diagnostic->charging.ShortCircuit.timer = 0;
            Relay_hold_off(); // 短路自检, 先断开主回路继电器
            break;
        case DIAGNOSTIC_CHG_SHORTCIRCUIT_CHECKING:
            Relay_hold_off(); // 短路自检, 先断开主回路继电器
            APP_DIAG_CHG_ShortCircuit_Check(&Diagnostic_Manager);
            if (Diagnostic->charging.ShortCircuit.timer >= 100) { // 100ms检测时间
                // 最终检查经过消抖后的状态
                DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT,DIOIF_PIN_LOW); //断开短路检测继电器
                if(true == APP_DIAG_CHG_ShortCircuit_GetFaultStatus(&Diagnostic_Manager)) {
                    st_event_fault_charging_data.type = E_CHARGING_FAULT_SHORT_CIRCUIT;
                    APP_CHG_EventPub(E_EVENT_CHG_FAULT, st_event_fault_charging_data.type);
                }
                Diagnostic->charging.ShortCircuit.currentState = DIAGNOSTIC_CHG_SHORTCIRCUIT_IDLE;
                Diagnostic->charging.ShortCircuit.timer = 0;
            }
            break;
        default:
            break;
    }
}

/********继电器黏连检测********/
/**
 * @brief 初始化继电器故障检测器
 * @param detector 检测器实例指针
 * @param faultDelayMs 故障确认持续时间(ms)
 * @param recoverDelayMs 故障恢复持续时间(ms)
 */
void APP_DIAG_CHG_RelayFault_Init(Diagnostic_Manager_t *Diagnostic,
                                  uint32_t faultDelayMs, uint32_t recoverDelayMs) {
    Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_NORMAL;
    Diagnostic->charging.Relay.faultDurationMs = faultDelayMs;
    Diagnostic->charging.Relay.recoverDurationMs = recoverDelayMs;
    Diagnostic->charging.Relay.faultCounterMs = 0;
    Diagnostic->charging.Relay.recoverCounterMs = 0;
    Diagnostic->charging.Relay.isFaultActive = false;
    Diagnostic->charging.Relay.lastRelayStatus = false;
}

/**
 * @brief 继电器故障检测状态机处理函数（在5ms定时器中调用）
 * @param detector 检测器实例指针
 * @param currentRelayStatus 当前继电器状态 (true:粘连, false:正常)
 * @return true 状态发生变化, false 状态无变化
 */
bool APP_DIAG_CHG_RelayFaultProcess(Diagnostic_Manager_t *Diagnostic, bool currentRelayStatus) {
    bool stateChanged = false;

    switch (Diagnostic->charging.Relay.currentState) {
        case DIAGNOSTIC_CHG_RELAY_STATE_NORMAL:
            if (currentRelayStatus) { // 检测到粘连
                Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_FAULT_DETECTING;
                Diagnostic->charging.Relay.faultCounterMs = 5; // 开始计时
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_CHG_RELAY_STATE_FAULT_DETECTING:
            if (currentRelayStatus) {
                // 持续检测到粘连
                Diagnostic->charging.Relay.faultCounterMs += 5;
                if (Diagnostic->charging.Relay.faultCounterMs >= Diagnostic->charging.Relay.faultDurationMs) {
                    // 达到故障确认时间, 进入故障状态
                    Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_FAULT;
                    Diagnostic->charging.Relay.isFaultActive = true;
                    stateChanged = true;
                }
            } else {
                // 粘连状态消失, 返回正常状态
                Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_NORMAL;
                Diagnostic->charging.Relay.faultCounterMs = 0;
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_CHG_RELAY_STATE_FAULT:
            if (!currentRelayStatus) { // 检测到恢复正常
                Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_RECOVERING;
                Diagnostic->charging.Relay.recoverCounterMs = 5; // 开始恢复计时
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_CHG_RELAY_STATE_RECOVERING:
            if (!currentRelayStatus) {
                // 持续正常状态
                Diagnostic->charging.Relay.recoverCounterMs += 5;
                if (Diagnostic->charging.Relay.recoverCounterMs >= Diagnostic->charging.Relay.recoverDurationMs) {
                    // 达到恢复确认时间, 返回正常状态
                    Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_NORMAL;
                    Diagnostic->charging.Relay.isFaultActive = false;
                    Diagnostic->charging.Relay.faultCounterMs = 0;
                    Diagnostic->charging.Relay.recoverCounterMs = 0;
                    stateChanged = true;
                }
            } else {
                // 再次检测到粘连, 返回故障状态
                Diagnostic->charging.Relay.currentState = DIAGNOSTIC_CHG_RELAY_STATE_FAULT;
                Diagnostic->charging.Relay.recoverCounterMs = 0;
                stateChanged = true;
            }
            break;
    }

    Diagnostic->charging.Relay.lastRelayStatus = currentRelayStatus;
    return stateChanged;
}

/**
 * @brief 获取当前故障状态
 * @param detector 检测器实例指针
 * @return true 故障激活, false 故障未激活
 */
bool APP_DIAG_CHG_isRelayFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.Relay.isFaultActive;
}

/**
 * @brief 获取当前状态机状态
 * @param detector 检测器实例指针
 * @return RelayFaultState 当前状态
 */
Diagnostic_Charging_RelayFaultState getRelayFaultState(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.Relay.currentState;
}

/**
 * @brief 系统保护控制函数
 * @param boardFault 板子故障状态
 * @param tipFault 枪头故障状态
 */
void APP_DIAG_CHG_HandleRelayProtection(bool relayFault) {
    static bool lastRelayFault = false;

    event_warning_charging_data_t st_event_warning_charging_data;
    event_warning_recovery_charging_data_t st_event_warning_recovery_charging_data;

    // 前置检查: 如果系统有其他更高优先级的故障, 且不是继电器黏连故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_RELAY_STICK)) {
        return; // 系统被其他故障锁定, 不处理接地故障
    }

    // 检查状态变化
    if (relayFault && !lastRelayFault) {
        if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus())
        {
            st_event_warning_charging_data.type = E_CHARGING_WARNING_RELAY_STICK;
            APP_CHG_EventPub(E_EVENT_CHG_WARNING, st_event_warning_charging_data.type);
        }
    }

    // 检查故障恢复
    if (!relayFault && lastRelayFault) {
        if(E_STATE_CHG_WARNING == APP_CHG_GetChargeStatus())
        {
            st_event_warning_recovery_charging_data.type = E_CHARGING_WARNING_RECOVERY_RELAY_STICK;
            APP_CHG_EventPub(E_EVENT_CHG_WARNING_RECOVERY, st_event_warning_recovery_charging_data.type);
        }
    }

    lastRelayFault = relayFault;
}

void APP_DIAG_CHG_RelayCheck(Diagnostic_Manager_t *Diagnostic)
{
    static uint16_t Rleay_tim5ms = 0;

    //继电器黏连保护在空闲、告警状态下保持检测
    if(E_STATE_CHG_WARNING == APP_CHG_GetChargeStatus())
    {
    }
    else if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus())
    {
        Rleay_tim5ms += 5;
        if(Rleay_tim5ms <= 1000) //增加拔枪动作后等待1s检测继电器粘连
        {
            return;
        }
        else
        {
            if(Rleay_tim5ms > 5000)
            {
                Rleay_tim5ms = 5000;
                return;
            }
        }
    }
    else
    {
        // 初始化继电器故障检测器: 故障确认1050ms, 故障恢复1000ms
        APP_DIAG_CHG_RelayFault_Init(&Diagnostic_Manager, 1050, 1000);
        Rleay_tim5ms = 0;
        return;
    }

    // 读取继电器黏连状态
    bool relayStatus = RTE_Call_AdcM_Vlotage(ADCMCFG_RelayCHK_CH);
    bool relayStateChanged = APP_DIAG_CHG_RelayFaultProcess(&Diagnostic_Manager, relayStatus);
    bool relayFault = APP_DIAG_CHG_isRelayFaultActive(&Diagnostic_Manager);

    #if 0
    // 如果有状态变化, 输出日志信息
    if (relayStateChanged) {
        rt_kprintf("DBG_INFO, DIAG", "[RELAY] State: %s\r\n", getRelayStateString(getRelayFaultState(&relayDetector)));
    }
    #endif

    // 处理系统保护
    APP_DIAG_CHG_HandleRelayProtection(relayFault);
}

/********过欠压检测********/
// 初始化过欠压保护状态机
void APP_DIAG_CHG_voltageProtector_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_NORMAL;
    Diagnostic->charging.Voltage.timerCount = 0;
    Diagnostic->charging.Voltage.alarmStatus = false;
    Diagnostic->charging.Voltage.lastVoltage = 0;
}

// 检测电压状态
Diagnostic_Charging_VoltageStatus checkVoltageStatus(uint32_t voltage) {
    if (voltage >= DIAGNOSTIC_CHG_OVER_VOLTAGE_THRESHOLD) {
        return DIAGNOSTIC_CHG_VOLTAGE_STATUS_OVER;
    } else if (voltage <= DIAGNOSTIC_CHG_UNDER_VOLTAGE_THRESHOLD) {
        return DIAGNOSTIC_CHG_VOLTAGE_STATUS_UNDER;
    } else if (voltage <= DIAGNOSTIC_CHG_OVER_VOLTAGE_RECOVERY && voltage >= DIAGNOSTIC_CHG_UNDER_VOLTAGE_RECOVERY) {
        return DIAGNOSTIC_CHG_VOLTAGE_STATUS_NORMAL;
    } else {
        return DIAGNOSTIC_CHG_VOLTAGE_STATUS_IN_RECOVERY_ZONE;
    }
}

// 状态机处理函数（在5ms定时器中调用）
void APP_DIAG_CHG_VoltageProtectionHandler(Diagnostic_Manager_t *Diagnostic, uint32_t currentVoltage) {
    Diagnostic_Charging_VoltageStatus voltageStatus = checkVoltageStatus(currentVoltage);
    Diagnostic->charging.Voltage.lastVoltage = currentVoltage;

    switch (Diagnostic->charging.Voltage.currentState) {
        case DIAGNOSTIC_CHG_VOLTAGE_STATE_NORMAL: {
            if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_OVER) {
                Diagnostic->charging.Voltage.timerCount++;
                if (Diagnostic->charging.Voltage.timerCount >= DIAGNOSTIC_CHG_ABNORMAL_TIME_THRESHOLD) {
                    // 过压持续1s, 切换到过压报警状态
                    Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_ALARM;
                    Diagnostic->charging.Voltage.alarmStatus = true;
                    Diagnostic->charging.Voltage.timerCount = 0;
                }
            } else if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_UNDER) {
                Diagnostic->charging.Voltage.timerCount++;
                if (Diagnostic->charging.Voltage.timerCount >= DIAGNOSTIC_CHG_ABNORMAL_TIME_THRESHOLD) {
                    // 欠压持续1s, 切换到欠压报警状态
                    Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_ALARM;
                    Diagnostic->charging.Voltage.alarmStatus = true;
                    Diagnostic->charging.Voltage.timerCount = 0;
                }
            } 
			else {
                // 电压正常, 重置计时器
                Diagnostic->charging.Voltage.timerCount = 0;
            }
            break;
        }

        case DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_ALARM: {
            if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_NORMAL) {
                // 电压恢复到正常范围, 开始恢复计时
                Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_RECOVERY;
                Diagnostic->charging.Voltage.timerCount = 0;
            }
            // 如果仍然是过压状态, 保持报警
            break;
        }

        case DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_ALARM: {
            if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_NORMAL) {
                // 电压恢复到正常范围，开始恢复计时
                Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_RECOVERY;
                Diagnostic->charging.Voltage.timerCount = 0;
            }
            // 如果仍然是欠压状态，保持报警
            break;
        }

        case DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_RECOVERY: {
            if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_NORMAL) {
                Diagnostic->charging.Voltage.timerCount++;
                if (Diagnostic->charging.Voltage.timerCount >= DIAGNOSTIC_CHG_RECOVERY_TIME_THRESHOLD) {
                    // 电压正常持续5s，恢复保护
                    Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_NORMAL;
                    Diagnostic->charging.Voltage.alarmStatus = false;
                    Diagnostic->charging.Voltage.timerCount = 0;
                }
            } else if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_OVER) {
                // 电压再次过压，返回到报警状态
                Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_ALARM;
                Diagnostic->charging.Voltage.timerCount = 0;
            }
            break;
        }

        case DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_RECOVERY: {
            if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_NORMAL) {
                Diagnostic->charging.Voltage.timerCount++;
                if (Diagnostic->charging.Voltage.timerCount >= DIAGNOSTIC_CHG_RECOVERY_TIME_THRESHOLD) {
                    // 电压正常持续5s，恢复保护
                    Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_NORMAL;
                    Diagnostic->charging.Voltage.alarmStatus = false;
                    Diagnostic->charging.Voltage.timerCount = 0;
                }
            } else if (voltageStatus == DIAGNOSTIC_CHG_VOLTAGE_STATUS_UNDER) {
                // 电压再次欠压，返回到报警状态
                Diagnostic->charging.Voltage.currentState = DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_ALARM;
                Diagnostic->charging.Voltage.timerCount = 0;
            }
            break;
        }
    }
}

// 获取报警状态
bool APP_DIAG_CHG_VoltageProtectionGetAlarmStatus(Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.Voltage.alarmStatus;
}

void APP_DIAG_CHG_HandleVoltageProtection(bool voltageFault) {
    static bool lastVoltageFault = false;

    event_protection_pause_charging_data_t st_event_protection_pause_charging_data;
    event_protection_recovery_charging_data_t st_event_protection_recovery_charging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是过欠压故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_OVER_UNDER_VOLTAGE)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (voltageFault && !lastVoltageFault) {
        //充电中过欠压触发保护暂停
        if(E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()){
            st_event_protection_pause_charging_data.type = E_CHARGING_PROTECTION_PAUSE_OVER_UNDER_VOLTAGE;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_PAUSE, st_event_protection_pause_charging_data.type);
        }
    }

    // 检查故障恢复
    if (!voltageFault && lastVoltageFault) {
        //过欠压恢复在保护暂停状态
        if(E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()){
            st_event_protection_recovery_charging_data.type = E_CHARGING_PROTECTION_RECOVERY_OVER_UNDER_VOLTAGE;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_RECOVERY,  st_event_protection_recovery_charging_data.type);
        }
    }

    lastVoltageFault = voltageFault;
}

void APP_DIAG_CHG_VoltageCheck(Diagnostic_Manager_t *Diagnostic)
{
    if((E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) ||
       (E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()))
    {

    }
    else
    {
        //过欠压
        APP_DIAG_CHG_voltageProtector_Init(&Diagnostic_Manager);
        return;
    }
    uint32_t currentVoltage = RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH);
    // 处理过欠压保护状态机
    APP_DIAG_CHG_VoltageProtectionHandler(&Diagnostic_Manager, currentVoltage);
    bool voltageFault = APP_DIAG_CHG_VoltageProtectionGetAlarmStatus(&Diagnostic_Manager);
    APP_DIAG_CHG_HandleVoltageProtection(voltageFault);
}

/****************************************过温检测****************************************/
/**
 * @brief 初始化温度故障检测器
 * @param detector 检测器实例指针
 * @param overheatThresh 过温阈值(℃)
 * @param cooldownThresh 恢复阈值(℃)
 * @param overheatDelayMs 过温延时(ms)
 * @param cooldownDelayMs 恢复延时(ms)
 */
void APP_DIAG_CHG_MCUTempFault_Init(Diagnostic_Manager_t *Diagnostic,
                                    uint32_t overheatThresh, uint32_t cooldownThresh,
                                    uint32_t overheatDelayMs, uint32_t cooldownDelayMs) {
    Diagnostic->charging.OverMCUTemp.currentState = DIAGNOSTIC_CHG_MCUTEMP_STATE_NORMAL;
    Diagnostic->charging.OverMCUTemp.overheatThreshold = overheatThresh;
    Diagnostic->charging.OverMCUTemp.cooldownThreshold = cooldownThresh;
    Diagnostic->charging.OverMCUTemp.overheatDurationMs = overheatDelayMs;
    Diagnostic->charging.OverMCUTemp.cooldownDurationMs = cooldownDelayMs;
    Diagnostic->charging.OverMCUTemp.overheatCounterMs = 0;
    Diagnostic->charging.OverMCUTemp.cooldownCounterMs = 0;
    Diagnostic->charging.OverMCUTemp.isFaultActive = false;
}

/**
 * @brief 温度故障检测状态机处理函数（在5ms定时器中调用）
 * @param detector 检测器实例指针
 * @param currentTemp 当前温度值(℃)
 * @return true 状态发生变化, false 状态无变化
 */
bool APP_DIAG_CHG_MCUTempFaultProcess(Diagnostic_Manager_t *Diagnostic, uint32_t currentTemp) 
{
	
    bool stateChanged = false;
    switch (Diagnostic->charging.OverMCUTemp.currentState) 
	{
        case DIAGNOSTIC_CHG_MCUTEMP_STATE_NORMAL:
            if (currentTemp > Diagnostic->charging.OverMCUTemp.overheatThreshold) 
			{
                Diagnostic->charging.OverMCUTemp.overheatCounterMs += 5; // 5ms定时周期

                if (Diagnostic->charging.OverMCUTemp.overheatCounterMs >= Diagnostic->charging.OverMCUTemp.overheatDurationMs) 
				{
                    // 进入故障状态
                    Diagnostic->charging.OverMCUTemp.currentState = DIAGNOSTIC_CHG_MCUTEMP_STATE_FAULT;
                    Diagnostic->charging.OverMCUTemp.isFaultActive = true;
                    stateChanged = true;
                }
            } 
			else
			{
                // 温度正常，重置过温计数器
                Diagnostic->charging.OverMCUTemp.overheatCounterMs = 0;
            }
            break;

        case DIAGNOSTIC_CHG_MCUTEMP_STATE_OVERHEATING:
            // 此示例中未使用OVERHEATING状态，可预留用于更复杂的状态机
            break;

        case DIAGNOSTIC_CHG_MCUTEMP_STATE_FAULT:
            if (currentTemp <= Diagnostic->charging.OverMCUTemp.cooldownThreshold) 
			{
                // 温度降到恢复阈值以下，进入冷却状态
                Diagnostic->charging.OverMCUTemp.currentState = DIAGNOSTIC_CHG_MCUTEMP_STATE_COOLING;
                Diagnostic->charging.OverMCUTemp.cooldownCounterMs = 0;
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_CHG_MCUTEMP_STATE_COOLING:
            if (currentTemp > Diagnostic->charging.OverMCUTemp.cooldownThreshold) 
			{
                // 温度再次升高，返回故障状态
                Diagnostic->charging.OverMCUTemp.currentState = DIAGNOSTIC_CHG_MCUTEMP_STATE_FAULT;
                Diagnostic->charging.OverMCUTemp.cooldownCounterMs = 0;
                stateChanged = true;
            } 
			else
			{
                Diagnostic->charging.OverMCUTemp.cooldownCounterMs += 5; // 5ms定时周期

                if (Diagnostic->charging.OverMCUTemp.cooldownCounterMs >= Diagnostic->charging.OverMCUTemp.cooldownDurationMs) 
				{
                    // 冷却完成，返回正常状态
                    Diagnostic->charging.OverMCUTemp.currentState = DIAGNOSTIC_CHG_MCUTEMP_STATE_NORMAL;
                    Diagnostic->charging.OverMCUTemp.isFaultActive = false;
                    Diagnostic->charging.OverMCUTemp.overheatCounterMs = 0;
                    Diagnostic->charging.OverMCUTemp.cooldownCounterMs = 0;
                    stateChanged = true;
                }
            }
            break;
    }
	return stateChanged;

}

/**
 * @brief 获取当前故障状态
 * @param detector 检测器实例指针
 * @return true 故障激活, false 故障未激活
 */
bool APP_DIAG_CHG_isMCUTempFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.OverMCUTemp.isFaultActive;
}

/**
 * @brief 获取当前状态机状态
 * @param detector 检测器实例指针
 * @return TempState 当前状态
 */
Diagnostic_Charging_MCUTempState APP_DIAG_CHG_MCUgetTempFaultState(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.OverMCUTemp.currentState;
}

/**
 * @brief 系统保护控制函数
 * @param boardFault 板子故障状态
 * @param tipFault 枪头故障状态
 */
void APP_DIAG_CHG_HandleOverMCUTempProtection(bool boardFault) {
    static bool lastBoardFault = false;

    event_protection_pause_charging_data_t st_event_protection_pause_charging_data;
    event_protection_recovery_charging_data_t st_event_protection_recovery_charging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是接地故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_OVER_MCUTEMPERATURE)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (boardFault && !lastBoardFault) {
        // 执行板子过温保护动作：降低功率、关闭输出等
        if(E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus())
        {
            st_event_protection_pause_charging_data.type = E_CHARGING_PROTECTION_PAUSE_OVER_MCUTEMPERATURE;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_PAUSE, st_event_protection_pause_charging_data.type);
        }
    }

    // 检查故障恢复
    if (!boardFault && lastBoardFault) {
        if(E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus())
        {
            st_event_protection_recovery_charging_data.type = E_CHARGING_PROTECTION_RECOVERY_OVER_MCUTEMPERATURE;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_RECOVERY, st_event_protection_recovery_charging_data.type);
        }
    }

    lastBoardFault = boardFault;
}

void APP_DIAG_CHG_MCUTempCheck(Diagnostic_Manager_t *Diagnostic)
{
    if((E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) ||
//       (E_STATE_CHG_VPAUSE == APP_CHG_GetChargeStatus()) ||
       (E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()))
    {

    }
    else
    {
        // 初始化板子温度检测器：过温95℃，恢复70℃，过温延时5000ms，恢复延时5000ms
        APP_DIAG_CHG_MCUTempFault_Init(&Diagnostic_Manager, 9500, 7000, 5000, 5000); //-40℃-105℃，底层计算提高50摄氏度，过温阈值95+50℃，过温恢复温度70+50摄氏度
        return;
    }

    int32_t boardTemperature = RTE_Call_AdcM_Vlotage(ADCMCFG_MCUTemp_CH);

    // 处理板子温度检测
    bool boardStateChanged = APP_DIAG_CHG_MCUTempFaultProcess(&Diagnostic_Manager, boardTemperature);
    bool boardFault = APP_DIAG_CHG_isMCUTempFaultActive(&Diagnostic_Manager);

#if 0
    // 如果有状态变化，输出日志信息
    if (boardStateChanged) {
        rt_kprintf("DBG_INFO, DIAG, [BOARD] State: %s, Temp: %.1f°C\n", getTempStateString(getTempFaultState(&boardTempDetector)),
                   boardTemperature);
    }
#endif

    // 处理系统保护（如果任一故障激活）
    APP_DIAG_CHG_HandleOverMCUTempProtection(boardFault);
}




/********************************放电-NTC1过温检测********************************/
/**
 * @brief 初始化温度故障检测器
 * @param detector 检测器实例指针
 * @param overheatThresh 过温阈值(℃)
 * @param cooldownThresh 恢复阈值(℃)
 * @param overheatDelayMs 过温延时(ms)
 * @param cooldownDelayMs 恢复延时(ms)
 */
void APP_DIAG_CHG_NTC1TempFault_Init(Diagnostic_Manager_t *Diagnostic,
                                      uint32_t overheatThresh, uint32_t cooldownThresh,
                                      uint32_t overheatDelayMs, uint32_t cooldownDelayMs) {
    Diagnostic->charging.OverNTC1Temp.currentState = DIAGNOSTIC_CHG_NTC1TEMP_STATE_NORMAL;
    Diagnostic->charging.OverNTC1Temp.overheatThreshold = overheatThresh;
    Diagnostic->charging.OverNTC1Temp.cooldownThreshold = cooldownThresh;
    Diagnostic->charging.OverNTC1Temp.overheatDurationMs = overheatDelayMs;
    Diagnostic->charging.OverNTC1Temp.cooldownDurationMs = cooldownDelayMs;
    Diagnostic->charging.OverNTC1Temp.overheatCounterMs = 0;
    Diagnostic->charging.OverNTC1Temp.cooldownCounterMs = 0;
    Diagnostic->charging.OverNTC1Temp.isFaultActive = false;
}

/**
 * @brief 温度故障检测状态机处理函数（在5ms定时器中调用）
 * @param detector 检测器实例指针
 * @param currentTemp 当前温度值(℃)
 * @return true 状态发生变化, false 状态无变化
 */
bool APP_DIAG_CHG_NTC1TempFaultProcess(Diagnostic_Manager_t *Diagnostic, int32_t currentTemp) {
    bool stateChanged = false;
    switch (Diagnostic->charging.OverNTC1Temp.currentState) {
        case DIAGNOSTIC_CHG_NTC1TEMP_STATE_NORMAL:
            if (currentTemp > Diagnostic->charging.OverNTC1Temp.overheatThreshold) {
                Diagnostic->charging.OverNTC1Temp.overheatCounterMs += 5; // 5ms定时周期

                if (Diagnostic->charging.OverNTC1Temp.overheatCounterMs >= Diagnostic->charging.OverNTC1Temp.overheatDurationMs) {
                    // 进入故障状态
                    Diagnostic->charging.OverNTC1Temp.currentState = DIAGNOSTIC_CHG_NTC1TEMP_STATE_FAULT;
                    Diagnostic->charging.OverNTC1Temp.isFaultActive = true;
                    stateChanged = true;
                }
            } else {
                // 温度正常，重置过温计数器
                Diagnostic->charging.OverNTC1Temp.overheatCounterMs = 0;
            }
            break;

        case DIAGNOSTIC_CHG_NTC1TEMP_STATE_OVERHEATING:
            // 此示例中未使用OVERHEATING状态，可预留用于更复杂的状态机
            break;

        case DIAGNOSTIC_CHG_NTC1TEMP_STATE_FAULT:
            if (currentTemp <= Diagnostic->charging.OverNTC1Temp.cooldownThreshold) {
                // 温度降到恢复阈值以下，进入冷却状态
                Diagnostic->charging.OverNTC1Temp.currentState = DIAGNOSTIC_CHG_NTC1TEMP_STATE_COOLING;
                Diagnostic->charging.OverNTC1Temp.cooldownCounterMs = 0;
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_CHG_NTC1TEMP_STATE_COOLING:
            if (currentTemp > Diagnostic->charging.OverNTC1Temp.cooldownThreshold) {
                // 温度再次升高，返回故障状态
                Diagnostic->charging.OverNTC1Temp.currentState = DIAGNOSTIC_CHG_NTC1TEMP_STATE_FAULT;
                Diagnostic->charging.OverNTC1Temp.cooldownCounterMs = 0;
                stateChanged = true;
            } else {
                Diagnostic->charging.OverNTC1Temp.cooldownCounterMs += 5; // 5ms定时周期

                if (Diagnostic->charging.OverNTC1Temp.cooldownCounterMs >= Diagnostic->charging.OverNTC1Temp.cooldownDurationMs) {
                    // 冷却完成，返回正常状态
                    Diagnostic->charging.OverNTC1Temp.currentState = DIAGNOSTIC_CHG_NTC1TEMP_STATE_NORMAL;
                    Diagnostic->charging.OverNTC1Temp.isFaultActive = false;
                    Diagnostic->charging.OverNTC1Temp.overheatCounterMs = 0;
                    Diagnostic->charging.OverNTC1Temp.cooldownCounterMs = 0;
                    stateChanged = true;
                }
            }
            break;
    }
    return stateChanged;
}

/**
 * @brief 获取当前故障状态
 * @param detector 检测器实例指针
 * @return true 故障激活, false 故障未激活
 */
bool APP_DIAG_CHG_isNTC1TempFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.OverNTC1Temp.isFaultActive;
}

/**
 * @brief 获取当前状态机状态
 * @param detector 检测器实例指针
 * @return TempState 当前状态
 */
Diagnostic_Charging_NTC1TempState APP_DIAG_CHG_getNTC1TempFaultState(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.OverNTC1Temp.currentState;
}

/**
 * @brief 系统保护控制函数
 * @param boardFault 板子故障状态
 * @param tipFault 枪头故障状态
 */
void APP_DIAG_CHG_HandleOverNTC1TempProtection(bool NTC1Fault) {
    static bool lastNTC1Fault = false;
    event_protection_pause_charging_data_t st_event_protection_pause_charging_data;
    event_protection_recovery_charging_data_t st_event_protection_recovery_charging_data;
    // 前置检查：如果系统有其他更高优先级的故障，且不是接地故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_OVER_NTC1TEMPERATURE)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (NTC1Fault && !lastNTC1Fault) {
        // 执行板子过温保护动作：降低功率、关闭输出等
        if (E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus())
        {
            st_event_protection_pause_charging_data.type = E_CHARGING_PROTECTION_PAUSE_OVER_NTC1TEMPERATURE;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_PAUSE, st_event_protection_pause_charging_data.type);
        }
    }

    // 检查故障恢复
    if (!NTC1Fault && lastNTC1Fault) {
        if (E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus())
        {
            st_event_protection_recovery_charging_data.type = E_CHARGING_PROTECTION_RECOVERY_OVER_NTC1TEMPERATURE;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_RECOVERY, st_event_protection_recovery_charging_data.type);
        }
    }
    lastNTC1Fault = NTC1Fault;
}

void APP_DIAG_CHG_NTC1TempCheck(Diagnostic_Manager_t *Diagnostic)
{
    if((E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) ||\
       (E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()))
    {

    } else {
        // 初始化板子温度检测器：过温105±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
        APP_DIAG_CHG_NTC1TempFault_Init(&Diagnostic_Manager, 100, 70, 5000, 5000);
        return;
    }
    int32_t NTC1Temperature = (int32_t)(RTE_Call_AdcM_Vlotage(ADCMCFG_NTC1_CH));
    // 处理板子温度检测
    bool NTC1StateChanged = APP_DIAG_CHG_NTC1TempFaultProcess(&Diagnostic_Manager, NTC1Temperature);
    bool NTC1Fault = APP_DIAG_CHG_isNTC1TempFaultActive(&Diagnostic_Manager);

    // 处理系统保护（如果任一故障激活）
    APP_DIAG_CHG_HandleOverNTC1TempProtection(NTC1Fault);
}


/****************************************接地异常****************************************/
//GroundFaultDiagnosis groundDiagnosis;

// 初始化接地不良诊断
void APP_DIAG_CHG_GroundFault_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_NORMAL;
    Diagnostic->charging.Ground.faultTimerCount = 0;
    Diagnostic->charging.Ground.recoveryTimerCount = 0;
    Diagnostic->charging.Ground.isFaultLocked = false;
    Diagnostic->charging.Ground.lastGroundStatus = false; // 默认假设接地良好
    Diagnostic->charging.Ground.faultTimeThreshold = DIAGNOSTIC_CHARGING_GROUND_FAULT_TIME_THRESHOLD;
    Diagnostic->charging.Ground.recoveryTimeThreshold = DIAGNOSTIC_CHARGING_GROUND_RECOVERY_TIME_THRESHOLD;
}

// 接地不良诊断处理函数
void APP_DIAG_CHG_GroundFaultHandler(Diagnostic_Manager_t *Diagnostic, uint32_t currentGroundStatus) {
    Diagnostic->charging.Ground.lastGroundStatus = currentGroundStatus;

    switch (Diagnostic->charging.Ground.currentState) {
        case DIAGNOSTIC_CHG_GROUND_STATE_NORMAL: {
            if (currentGroundStatus == 1) { // 接地不良
                // 第一次检测到接地不良，进入故障检测中状态
                Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_FAULT_DETECTING;
                Diagnostic->charging.Ground.faultTimerCount = 1; // 开始计时
            } else {
                // 接地良好，确保状态正常
                Diagnostic->charging.Ground.faultTimerCount = 0;
                Diagnostic->charging.Ground.recoveryTimerCount = 0;
            }
            break;
        }

        case DIAGNOSTIC_CHG_GROUND_STATE_FAULT_DETECTING: {
            if (currentGroundStatus == 1) { // 仍然接地不良
                Diagnostic->charging.Ground.faultTimerCount++;
                if (Diagnostic->charging.Ground.faultTimerCount >= Diagnostic->charging.Ground.faultTimeThreshold) {
                    // 接地不良持续设定时间，切换到故障锁定状态
                    Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_FAULT_LOCKED;
                    Diagnostic->charging.Ground.isFaultLocked = true;
                    Diagnostic->charging.Ground.faultTimerCount = 0;
                }
            } else {
                // 在检测过程中接地恢复正常，切换回正常状态
                Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_NORMAL;
                Diagnostic->charging.Ground.faultTimerCount = 0;
            }
            break;
        }

        case DIAGNOSTIC_CHG_GROUND_STATE_FAULT_LOCKED: {

            if (currentGroundStatus == 0) { // 接地良好
                // 第一次检测到接地良好，进入恢复中状态
                Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_RECOVERING;
                Diagnostic->charging.Ground.recoveryTimerCount = 1; // 开始恢复计时
            } else {
                // 仍然接地不良，保持故障锁定状态
                Diagnostic->charging.Ground.recoveryTimerCount = 0;
            }
            break;
        }

        case DIAGNOSTIC_CHG_GROUND_STATE_RECOVERING: {
            if (currentGroundStatus == 0) { // 接地良好
                Diagnostic->charging.Ground.recoveryTimerCount++;
                if (Diagnostic->charging.Ground.recoveryTimerCount >= Diagnostic->charging.Ground.recoveryTimeThreshold) {
                    // 接地良好持续5秒，故障恢复
                    Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_NORMAL;
                    Diagnostic->charging.Ground.isFaultLocked = false;
                    Diagnostic->charging.Ground.recoveryTimerCount = 0;
                    Diagnostic->charging.Ground.faultTimerCount = 0;
                }
            } else {
                // 恢复过程中再次出现接地不良，返回故障锁定状态
                Diagnostic->charging.Ground.currentState = DIAGNOSTIC_CHG_GROUND_STATE_FAULT_LOCKED;
                Diagnostic->charging.Ground.recoveryTimerCount = 0;
            }
            break;
        }
    }
}

bool APP_DIAG_CHG_GroundFaultIsLocked(Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.Ground.isFaultLocked;
}

void APP_DIAG_CHG_HandleGroundProtection(bool groundFault) {
    static bool lastGroundFault = false;

    event_warning_charging_data_t st_event_warning_charging_data;
    event_warning_recovery_charging_data_t st_event_warning_recovery_charging_data;
//    event_fault_charging_data_t st_event_fault_charging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是接地故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_GROUND_FAULT)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (groundFault && !lastGroundFault) {

        //接地保护在空闲、插枪进入告警
        if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus())
        {
            st_event_warning_charging_data.type = E_CHARGING_WARNING_GROUND_WARNING;
            APP_CHG_EventPub(E_EVENT_CHG_WARNING, st_event_warning_charging_data.type);
        }
//只有在送检测试过程中开启该功能
//        else if(E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) //充电中状态进入故障
//        {
//            rt_kprintf("DBG_INFO, DIAG, E_FAULT_GROUND_FAULT\r\n");
//            st_event_fault_charging_data.type = E_CHARGING_FAULT_GROUND_FAULT;
//            APP_CHG_EventPub(E_EVENT_CHG_FAULT, st_event_fault_charging_data.type);
//        }
        else
        {

        }
    }

    // 检查故障恢复
    if (!groundFault && lastGroundFault) {

        //接地保护恢复-告警状态
        if(E_STATE_CHG_WARNING == APP_CHG_GetChargeStatus())
        {
            st_event_warning_recovery_charging_data.type = E_CHARGING_WARNING_RECOVERY_GROUND_WARNING;
            APP_CHG_EventPub(E_EVENT_CHG_WARNING_RECOVERY, st_event_warning_recovery_charging_data.type);
        }
        else
        {

        }
    }

    lastGroundFault = groundFault;
}

void APP_DIAG_CHG_GroundCheck(Diagnostic_Manager_t *Diagnostic)
{
    //接地保护在空闲、插枪、告警、充电中、保护暂停状态下保持检测
    if(E_STATE_CHG_IDLE == APP_CHG_GetChargeStatus() || \
       (E_STATE_CHG_CONNECTED == APP_CHG_GetChargeStatus()) ||\
       (E_STATE_CHG_WARNING == APP_CHG_GetChargeStatus()) ||\
       (E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) ||\
       (E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()))
    {

    }
    else
    {
        //非检测状态重新初始化一下
        APP_DIAG_CHG_GroundFault_Init(&Diagnostic_Manager);
        return;
    }

    // 读取API接口获取接地状态
    uint32_t currentGroundStatus = RTE_Call_AdcM_Vlotage(ADCMCFG_Ground_CH);

    // 处理接地不良诊断
    APP_DIAG_CHG_GroundFaultHandler(&Diagnostic_Manager, currentGroundStatus);

    bool groundFault = APP_DIAG_CHG_GroundFaultIsLocked(&Diagnostic_Manager);

    // 处理系统保护
    APP_DIAG_CHG_HandleGroundProtection(groundFault);
}

/****************************************过流****************************************/
//OvercurrentProtector ocProtector;

/**
 * @brief 初始化过流保护器
 * @param protector 保护器实例指针
 */
void APP_DIAG_CHG_CurProtector_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL;

    // 初始化配置参数
    Diagnostic->charging.Overcurrent.config.cpSetCurrent = Get_Adapter_detection_current(&st_Adapter_state_machine);
    Diagnostic->charging.Overcurrent.config.level1Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent+2)*1000;    // 一段固定10-12A
    Diagnostic->charging.Overcurrent.config.level2Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent+4)*1000;    // 二段固定12-18A
    Diagnostic->charging.Overcurrent.config.level3Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent+10)*1000;   // 三段 >= 18A
    Diagnostic->charging.Overcurrent.config.level1DurationMs = 5000;               // 5秒
    Diagnostic->charging.Overcurrent.config.level2DurationMs = 3000;               // 3秒
    Diagnostic->charging.Overcurrent.config.level3DurationMs = 10;                 // 10ms
    Diagnostic->charging.Overcurrent.config.recoveryDelayMs = 30000;               // 30秒恢复时间
    Diagnostic->charging.Overcurrent.config.recoveryCheckMs = 2000;                // 2秒恢复检测时间
    Diagnostic->charging.Overcurrent.config.maxLevel1Faults = 3;                   // 最大3次

    // 初始化恢复检测器
    Diagnostic->charging.Overcurrent.recovery.state = DIAGNOSTIC_CHG_OC_RECOVERY_IDLE;
    Diagnostic->charging.Overcurrent.recovery.checkCounterMs = 0;
    Diagnostic->charging.Overcurrent.recovery.countdownCounterMs = 0;
    Diagnostic->charging.Overcurrent.recovery.conditionMet = false;
    Diagnostic->charging.Overcurrent.recovery.currentThreshold = (float)ADCM_t.cali.current.cail_b/10;

    // 初始化运行数据
    Diagnostic->charging.Overcurrent.level1CounterMs = 0;
    Diagnostic->charging.Overcurrent.level2CounterMs = 0;
    Diagnostic->charging.Overcurrent.level1FaultCount = 0;
    Diagnostic->charging.Overcurrent.isFaultActive = false;
    Diagnostic->charging.Overcurrent.isChargingLocked = false;
    Diagnostic->charging.Overcurrent.currentCurrent = 0.0f;
}

void APP_DIAG_CHG_CurProtector_Init2(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL;

    // 初始化配置参数
    Diagnostic->charging.Overcurrent.config.cpSetCurrent = Get_Adapter_detection_current(&st_Adapter_state_machine);
    Diagnostic->charging.Overcurrent.config.level1Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent+2)*1000;    // 一段固定10-12A
    Diagnostic->charging.Overcurrent.config.level2Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent+4)*1000;    // 二段固定12-18A
    Diagnostic->charging.Overcurrent.config.level3Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent+10)*1000;   // 三段 >= 18A
    Diagnostic->charging.Overcurrent.config.level1DurationMs = 5000;               // 5秒
    Diagnostic->charging.Overcurrent.config.level2DurationMs = 3000;               // 3秒
    Diagnostic->charging.Overcurrent.config.level3DurationMs = 10;                 // 10ms
    Diagnostic->charging.Overcurrent.config.recoveryDelayMs = 30000;               // 30秒恢复时间
    Diagnostic->charging.Overcurrent.config.recoveryCheckMs = 2000;                // 2秒恢复检测时间
    Diagnostic->charging.Overcurrent.config.maxLevel1Faults = 3;                   // 最大3次

    // 初始化恢复检测器
    Diagnostic->charging.Overcurrent.recovery.state = DIAGNOSTIC_CHG_OC_RECOVERY_IDLE;
    Diagnostic->charging.Overcurrent.recovery.checkCounterMs = 0;
    Diagnostic->charging.Overcurrent.recovery.countdownCounterMs = 0;
    Diagnostic->charging.Overcurrent.recovery.conditionMet = false;
    Diagnostic->charging.Overcurrent.recovery.currentThreshold = (float)ADCM_t.cali.current.cail_b/10;

    // 初始化运行数据
    Diagnostic->charging.Overcurrent.level1CounterMs = 0;
    Diagnostic->charging.Overcurrent.level2CounterMs = 0;
//    Diagnostic->charging.Overcurrent.level1FaultCount = 0;
    Diagnostic->charging.Overcurrent.isFaultActive = false;
    Diagnostic->charging.Overcurrent.isChargingLocked = false;
    Diagnostic->charging.Overcurrent.currentCurrent = 0.0f;
}

void APP_DIAG_CHG_CurProtector_Set(Diagnostic_Manager_t *Diagnostic)
{
    Diagnostic->charging.Overcurrent.config.cpSetCurrent = Get_Adapter_detection_current(&st_Adapter_state_machine); // 获取CP电流
    Diagnostic->charging.Overcurrent.config.level1Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent + 2) * 1000; // 一级阈值
    Diagnostic->charging.Overcurrent.config.level2Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent + 4) * 1000; // 二级阈值
    Diagnostic->charging.Overcurrent.config.level3Threshold = (Diagnostic->charging.Overcurrent.config.cpSetCurrent + 10) * 1000; // 三级阈值
}


/**
 * @brief 重置恢复检测器
 * @param Diagnostic->charging.Overcurrent. 保护器实例指针
 */
static void APP_DIAG_CHG_CurresetRecovery(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.Overcurrent.recovery.state = DIAGNOSTIC_CHG_OC_RECOVERY_IDLE;
    Diagnostic->charging.Overcurrent.recovery.checkCounterMs = 0;
    Diagnostic->charging.Overcurrent.recovery.countdownCounterMs = 0;
    Diagnostic->charging.Overcurrent.recovery.conditionMet = false;
}

/**
 * @brief 过流保护处理函数（5ms定时器调用）
 * @param Diagnostic->charging.Overcurrent. 保护器实例指针
 * @param measuredCurrent 测量电流值(A)
 */
void APP_DIAG_CHG_CurProcess(Diagnostic_Manager_t *Diagnostic, uint32_t measuredCurrent) {
    Diagnostic->charging.Overcurrent.currentCurrent = measuredCurrent;

    switch (Diagnostic->charging.Overcurrent.currentState) {
        case DIAGNOSTIC_CHG_OC_STATE_NORMAL:
            APP_DIAG_CHG_CurresetRecovery(Diagnostic);

            if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level3Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL3_FAULT;
                Diagnostic->charging.Overcurrent.level3CounterMs = 5;
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level2Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL2_FAULT;
                Diagnostic->charging.Overcurrent.level2CounterMs = 5;
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level1Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL1_DETECTING;
                Diagnostic->charging.Overcurrent.level1CounterMs = 5;
            }
            break;

        case DIAGNOSTIC_CHG_OC_STATE_LEVEL1_DETECTING:
            if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level3Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL3_FAULT;
                Diagnostic->charging.Overcurrent.level3CounterMs = 5;
                Diagnostic->charging.Overcurrent.level2CounterMs = 0;
                Diagnostic->charging.Overcurrent.level1CounterMs = 0;
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level2Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL2_FAULT;
                Diagnostic->charging.Overcurrent.level3CounterMs = 0;
                Diagnostic->charging.Overcurrent.level2CounterMs = 5;
                Diagnostic->charging.Overcurrent.level1CounterMs = 0;
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level1Threshold) {
                Diagnostic->charging.Overcurrent.level1CounterMs += 5;
                if (Diagnostic->charging.Overcurrent.level1CounterMs >= Diagnostic->charging.Overcurrent.config.level1DurationMs) {
                    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL1_FAULT;
                    Diagnostic->charging.Overcurrent.isFaultActive = true;
                    Diagnostic->charging.Overcurrent.level1FaultCount++;
                    APP_DIAG_CHG_CurresetRecovery(Diagnostic);
                }
            }
            else {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL;
                Diagnostic->charging.Overcurrent.level1CounterMs = 0;
            }
            break;

        case DIAGNOSTIC_CHG_OC_STATE_LEVEL1_FAULT:
            // 恢复条件检测
            if (Diagnostic->charging.Overcurrent.level1FaultCount >= Diagnostic->charging.Overcurrent.config.maxLevel1Faults) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LOCKED;
                Diagnostic->charging.Overcurrent.isChargingLocked = true;
            }

            if (measuredCurrent <= Diagnostic->charging.Overcurrent.recovery.currentThreshold)
            {
                if (Diagnostic->charging.Overcurrent.recovery.state == DIAGNOSTIC_CHG_OC_RECOVERY_IDLE) {
                    Diagnostic->charging.Overcurrent.recovery.state = DIAGNOSTIC_CHG_OC_RECOVERY_CHECKING;
                }

                Diagnostic->charging.Overcurrent.recovery.checkCounterMs += 5;

                if (Diagnostic->charging.Overcurrent.recovery.checkCounterMs >= Diagnostic->charging.Overcurrent.config.recoveryCheckMs) { //检测到0A持续2s后开始30s倒计时
                    Diagnostic->charging.Overcurrent.recovery.state = DIAGNOSTIC_CHG_OC_RECOVERY_CONDITION_MET;
                    Diagnostic->charging.Overcurrent.recovery.conditionMet = true;
                }
            }
            else {
                APP_DIAG_CHG_CurresetRecovery(Diagnostic);
                Diagnostic->charging.Overcurrent.recovery.countdownCounterMs = 0;
            }

            // 恢复倒计时
            if (Diagnostic->charging.Overcurrent.recovery.conditionMet) {
                Diagnostic->charging.Overcurrent.recovery.state = DIAGNOSTIC_CHG_OC_RECOVERY_COUNTDOWN;
                Diagnostic->charging.Overcurrent.recovery.countdownCounterMs += 5;

                if (Diagnostic->charging.Overcurrent.recovery.countdownCounterMs >= Diagnostic->charging.Overcurrent.config.recoveryDelayMs) {
                    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL;
                    Diagnostic->charging.Overcurrent.isFaultActive = false;
                    APP_DIAG_CHG_CurresetRecovery(Diagnostic);
                }
            }
            break;

        case DIAGNOSTIC_CHG_OC_STATE_LEVEL2_FAULT:
            APP_DIAG_CHG_CurresetRecovery(Diagnostic);
            if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level3Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL3_FAULT;
                Diagnostic->charging.Overcurrent.level3CounterMs = 5;
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level2Threshold) {
                Diagnostic->charging.Overcurrent.level2CounterMs += 5;
                if (Diagnostic->charging.Overcurrent.level2CounterMs >= Diagnostic->charging.Overcurrent.config.level2DurationMs) {
                    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LOCKED;
                    Diagnostic->charging.Overcurrent.isFaultActive = true;
                    Diagnostic->charging.Overcurrent.isChargingLocked = true;
                }
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level1Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL1_DETECTING;
                Diagnostic->charging.Overcurrent.level1CounterMs = 5;
            }
            else {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL; //持续3s判定为二段过流，3s内检测低于2段过流值重新判定
                Diagnostic->charging.Overcurrent.level3CounterMs = 0;
                Diagnostic->charging.Overcurrent.level2CounterMs = 0;
            }
            break;

        case DIAGNOSTIC_CHG_OC_STATE_LEVEL3_FAULT:
            APP_DIAG_CHG_CurresetRecovery(Diagnostic);

            if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level3Threshold) {
                Diagnostic->charging.Overcurrent.level3CounterMs += 5;

                if (Diagnostic->charging.Overcurrent.level3CounterMs >= Diagnostic->charging.Overcurrent.config.level3DurationMs) {
                    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LOCKED;
                    Diagnostic->charging.Overcurrent.isFaultActive = true;
                    Diagnostic->charging.Overcurrent.isChargingLocked = true;
                }
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level2Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL2_FAULT;
                Diagnostic->charging.Overcurrent.level2CounterMs = 5;
            }
            else if (measuredCurrent > Diagnostic->charging.Overcurrent.config.level1Threshold) {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_LEVEL1_DETECTING;
                Diagnostic->charging.Overcurrent.level1CounterMs = 5;
            }
            else {
                Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL;
                Diagnostic->charging.Overcurrent.level3CounterMs = 0;
                Diagnostic->charging.Overcurrent.level2CounterMs = 0;
            }
            break;

        case DIAGNOSTIC_CHG_OC_STATE_LOCKED:
            // 一段过流拔枪可恢复，二段、三段过流重新上电恢复
//            rt_kprintf("DIAGNOSTIC_CHG_OC_STATE_LOCKED\r\n");
            break;
    }
}

// 其他函数实现
bool APP_DIAG_CHG_CurIsFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.Overcurrent.isFaultActive;
}

bool APP_DIAG_CHG_CurIsChargingLocked(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->charging.Overcurrent.isChargingLocked;
}

void APP_DIAG_CHG_CurReset(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->charging.Overcurrent.currentState = DIAGNOSTIC_CHG_OC_STATE_NORMAL;
    Diagnostic->charging.Overcurrent.level1CounterMs = 0;
    Diagnostic->charging.Overcurrent.level2CounterMs = 0;
    Diagnostic->charging.Overcurrent.level1FaultCount = 0;
    Diagnostic->charging.Overcurrent.isFaultActive = false;
    Diagnostic->charging.Overcurrent.isChargingLocked = false;
    APP_DIAG_CHG_CurresetRecovery(Diagnostic);
}

//void APP_DIAG_CHG_OCReset(Diagnostic_Manager_t *Diagnostic)
//{
//    APP_DIAG_CHG_CurReset(&Diagnostic_Manager);
//}

void APP_DIAG_CHG_CurHandleOverCurrentProtection(bool currentFault, bool currentLocked)
{
    static bool lastFaultState = false;
    static bool lastLockedState = false;

    event_fault_charging_data_t st_event_fault_charging_data;
    event_protection_pause_charging_data_t st_event_protection_pause_charging_data;
    event_protection_recovery_charging_data_t st_event_protection_recovery_charging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是过流故障引起的
    if (APP_CHG_IsAnyFaultPresent() && !APP_CHG_IsFaultPresent(E_CHARGING_FAULT_OVER_CURRENT)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检测状态变化
    bool faultChanged = (currentFault != lastFaultState);
    bool lockedChanged = (currentLocked != lastLockedState);

    if (!faultChanged && !lockedChanged) {
        return; // 状态无变化，不处理事件
    }

    // 判断事件类型
    if (currentLocked) {
        // 二段过流锁死或者是一段过流超3次
        if(E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus())
        {
            st_event_fault_charging_data.type = E_CHARGING_FAULT_OVER_CURRENT;
            APP_CHG_EventPub(E_EVENT_CHG_FAULT,  st_event_fault_charging_data.type);
        }
    }
    else if (currentFault) {
        // 一段过流保护
        if(E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus())
        {
            st_event_protection_pause_charging_data.type = E_CHARGING_PROTECTION_PAUSE_CURRENT;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_PAUSE, st_event_protection_pause_charging_data.type);
        }
    }
    else if (lastFaultState && !currentFault) {
        // 一段过流恢复
        if(E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus())
        {
            st_event_protection_recovery_charging_data.type = E_CHARGING_PROTECTION_RECOVERY_CURRENT;
            APP_CHG_EventPub(E_EVENT_CHG_PROTECTION_RECOVERY, st_event_protection_recovery_charging_data.type);
        }
    }

    lastFaultState = currentFault;
    lastLockedState = currentLocked;
}

void APP_DIAG_CHG_OverCurrentCheck(Diagnostic_Manager_t *Diagnostic)
{
//过流保护在充电中、保护暂停状态下保持检测
    if((E_STATE_CHG_CHARGING == APP_CHG_GetChargeStatus()) ||\
       (E_STATE_CHG_PPAUSE == APP_CHG_GetChargeStatus()))
    {

    }
    else
    {
        // 初始化过流保护器
        APP_DIAG_CHG_CurProtector_Init2(&Diagnostic_Manager);
        return;
    }

    uint32_t current = RTE_Call_AdcM_Vlotage(ADCMCFG_Currnet_CH);
    APP_DIAG_CHG_CurProcess(&Diagnostic_Manager, current);
    bool currentFault = APP_DIAG_CHG_CurIsFaultActive(&Diagnostic_Manager);
    bool currentLocked = APP_DIAG_CHG_CurIsChargingLocked(&Diagnostic_Manager);

    APP_DIAG_CHG_CurHandleOverCurrentProtection(currentFault, currentLocked);
}

/****************************************放电诊断部分****************************************/
/********漏电检测 1ms执行一次********/
void APP_DIAG_DISCHG_LeakageCheck_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->discharging.LeakageCheck.currentState = DIAGNOSTIC_DISCHG_LeakageCheck_IDLE;
    Diagnostic->discharging.LeakageCheck.count = 0;
}

void APP_DIAG_DISCHG_LeakageCheck_Start(Diagnostic_Manager_t *Diagnostic)
{
    Diagnostic->discharging.LeakageCheck.currentState = DIAGNOSTIC_DISCHG_LeakageCheck_START;
}

void APP_DIAG_DISCHG_LeakageCheck(Diagnostic_Manager_t *Diagnostic)
{
    event_fault_discharging_data_t st_event_fault_discharging_data;

    if(Diagnostic->discharging.LeakageCheck.currentState != DIAGNOSTIC_DISCHG_LeakageCheck_START)
    {
        return;
    }

    if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus())
    {

    }
    else
    {
        APP_DIAG_DISCHG_LeakageCheck_Init(Diagnostic);
        return;
    }

    // 前置检查：如果系统有其他更高优先级的故障，且不是漏电故障引起的
    if (APP_DISCHG_IsAnyFaultPresent() && !APP_DISCHG_IsFaultPresent(E_DISCHARGING_FAULT_LEAKAGE)) 
	{
        return; // 系统被其他故障锁定,不处理
    }

    // 1. 读取硬件IO或ADC值
    if(ADCM_t.debug.leakage.flag)
    {
        Diagnostic->discharging.LeakageCheck.Leakage_SCR_Status = ADCM_t.debug.leakage.leakagestatus;
    }
    else
    {
        Diagnostic->discharging.LeakageCheck.Leakage_SCR_Status = RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT);
    }

    if(Diagnostic->discharging.LeakageCheck.Leakage_SCR_Status == DIOIF_PIN_HIGH)
    {
        Diagnostic->discharging.LeakageCheck.count += 1;
    }
    else
    {
        Diagnostic->discharging.LeakageCheck.count = 0; // 一旦发现没问题，计数器清零
    }

    // 3. 更新最终状态
    if(Diagnostic->discharging.LeakageCheck.count >= 5)
    {
        st_event_fault_discharging_data.type = E_DISCHARGING_FAULT_LEAKAGE;
        APP_DISCHG_EventPub(E_EVENT_DISCHG_FAULT, st_event_fault_discharging_data.type);
    }
}

/********放电-过欠压********/
// 初始化过欠压保护状态机
void APP_DIAG_DISCHG_VoltageProtection_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_NORMAL;
    Diagnostic->discharging.Voltage.timerCount = 0;
    Diagnostic->discharging.Voltage.OvermaxFaults = 3;
    Diagnostic->discharging.Voltage.UndermaxFaults = 3;
    Diagnostic->discharging.Voltage.OverFaultCount = 0;
    Diagnostic->discharging.Voltage.UnderFaultCount = 0;
    Diagnostic->discharging.Voltage.alarmStatus = false;
    Diagnostic->discharging.Voltage.lastVoltage = 0;
}

// 检测电压状态
Diagnostic_Discharging_VoltageStatus APP_DIAG_DISCHG_checkVoltageStatus(uint32_t voltage) {
    if (voltage >= DIAGNOSTIC_DISCHG_OVER_VOLTAGE_THRESHOLD) {
        return DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_OVER;
    } else if (voltage <= DIAGNOSTIC_DISCHG_UNDER_VOLTAGE_THRESHOLD) {
        return DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_UNDER;
    } else if (voltage <= DIAGNOSTIC_DISCHG_OVER_VOLTAGE_RECOVERY && voltage >= DIAGNOSTIC_DISCHG_UNDER_VOLTAGE_RECOVERY) {
        return DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_NORMAL;
    } else {
        return DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_IN_RECOVERY_ZONE;
    }
}

// 状态机处理函数（在5ms定时器中调用）
void APP_DIAG_DISCHG_VoltageProtectionHandler(Diagnostic_Manager_t *Diagnostic, uint32_t currentVoltage) {
    Diagnostic_Discharging_VoltageStatus voltageStatus = APP_DIAG_DISCHG_checkVoltageStatus(currentVoltage);
    Diagnostic->discharging.Voltage.lastVoltage = currentVoltage;

    switch (Diagnostic->discharging.Voltage.currentState) {
        case DIAGNOSTIC_DISCHG_VOLTAGE_STATE_NORMAL:
            if (voltageStatus == DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_OVER) {
                Diagnostic->discharging.Voltage.timerCount++;
                if (Diagnostic->discharging.Voltage.timerCount >= DIAGNOSTIC_DISCHG_ABNORMAL_TIME_THRESHOLD) {
                    // 过压持续1s，切换到过压报警状态
                    Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_OVER_ALARM;
                    Diagnostic->discharging.Voltage.OverFaultCount += 1;
                    Diagnostic->discharging.Voltage.alarmStatus = true;
                    Diagnostic->discharging.Voltage.timerCount = 0;
                }
            }
            else if (voltageStatus == DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_UNDER) {
                Diagnostic->discharging.Voltage.timerCount++;
                if (Diagnostic->discharging.Voltage.timerCount >= DIAGNOSTIC_DISCHG_ABNORMAL_TIME_THRESHOLD) {
                    // 欠压持续1s，切换到欠压报警状态
                    Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_UNDER_ALARM;
                    Diagnostic->discharging.Voltage.UnderFaultCount += 1;
                    Diagnostic->discharging.Voltage.alarmStatus = true;
                    Diagnostic->discharging.Voltage.timerCount = 0;
                }
            }
            else {
                // 电压正常，重置计时器
                Diagnostic->discharging.Voltage.timerCount = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_VOLTAGE_STATE_OVER_ALARM:
            // 电压恢复到正常范围，开始恢复计时
            Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_OVER_RECOVERY;
            Diagnostic->discharging.Voltage.timerCount = 0;
            // 如果仍然是过压状态，保持报警
            break;

        case DIAGNOSTIC_DISCHG_VOLTAGE_STATE_UNDER_ALARM:
            // 电压恢复到正常范围，开始恢复计时
            Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_UNDER_RECOVERY;
            Diagnostic->discharging.Voltage.timerCount = 0;
            // 如果仍然是欠压状态，保持报警
            break;

        case DIAGNOSTIC_DISCHG_VOLTAGE_STATE_OVER_RECOVERY:
            if (Diagnostic->discharging.Voltage.OverFaultCount >= Diagnostic->discharging.Voltage.OvermaxFaults) { //故障触发后30s检测一次，单次上电故障超过三次，进入锁死状态
                Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_LOCKED;
                Diagnostic->discharging.Voltage.alarmStatus = true;
                Diagnostic->discharging.Voltage.timerCount = 0;
            }
            Diagnostic->discharging.Voltage.timerCount++;
            if (Diagnostic->discharging.Voltage.timerCount >= DIAGNOSTIC_DISCHG_RECOVERY_TIME_THRESHOLD) {
                Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_NORMAL;
                Diagnostic->discharging.Voltage.alarmStatus = false;
                Diagnostic->discharging.Voltage.timerCount = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_VOLTAGE_STATE_UNDER_RECOVERY:
            if (Diagnostic->discharging.Voltage.UnderFaultCount >= Diagnostic->discharging.Voltage.UndermaxFaults) { //故障触发后30s检测一次，单次上电故障超过三次，进入锁死状态
                Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_LOCKED;
                Diagnostic->discharging.Voltage.alarmStatus = true;
                Diagnostic->discharging.Voltage.timerCount = 0;
            }
            Diagnostic->discharging.Voltage.timerCount++;
            if (Diagnostic->discharging.Voltage.timerCount >= DIAGNOSTIC_DISCHG_RECOVERY_TIME_THRESHOLD) {
                Diagnostic->discharging.Voltage.currentState = DIAGNOSTIC_DISCHG_VOLTAGE_STATE_NORMAL;
                Diagnostic->discharging.Voltage.alarmStatus = false;
                Diagnostic->discharging.Voltage.timerCount = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_VOLTAGE_STATE_LOCKED:
            break;
        default:
            break;
    }
}

// 获取报警状态
bool APP_DIAG_DISCHG_VoltageProtectionGetAlarmStatus(Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.Voltage.alarmStatus;
}

void APP_DIAG_DISCHG_HandleVoltageProtection(bool voltageFault) {
    static bool lastVoltageFault = false;

    event_protection_pause_discharging_data_t st_event_protection_pause_discharging_data;
    event_protection_recovery_discharging_data_t st_event_protection_recovery_discharging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是过欠压故障引起的
    if (APP_DISCHG_IsAnyFaultPresent() && !APP_DISCHG_IsFaultPresent(E_DISCHARGING_FAULT_OVER_UNDER_VOLTAGE)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (voltageFault && !lastVoltageFault) {
        //充电中过欠压触发保护暂停
        if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus()){
            st_event_protection_pause_discharging_data.type = E_DISCHARGING_PROTECTION_PAUSE_OVER_UNDER_VOLTAGE;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_PAUSE, st_event_protection_pause_discharging_data.type);
        }
    }

    // 检查故障恢复
    if (!voltageFault && lastVoltageFault) {
        //过欠压恢复在保护暂停状态
        if(E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus()){
            st_event_protection_recovery_discharging_data.type = E_DISCHARGING_PROTECTION_RECOVERY_OVER_UNDER_VOLTAGE;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_RECOVERY,  st_event_protection_recovery_discharging_data.type);
        }
    }

    lastVoltageFault = voltageFault;
}

void APP_DIAG_DISCHG_VoltageCheck(Diagnostic_Manager_t *Diagnostic)
{
    if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus() ||
       E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus())
    {
        if(APP_DISCHG_IsAnyFaultPresent() && !APP_DISCHG_IsFaultPresent(E_DISCHARGING_FAULT_OVER_UNDER_VOLTAGE))
        {
            return; //存在其他故障时继电器断开，电压为0，故存在其他故障时不做过欠压检测
        }
    }
    else
    {
        APP_DIAG_DISCHG_VoltageProtection_Init(&Diagnostic_Manager);
        return;
    }
    uint32_t currentVoltage = RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH);
    // 处理过欠压保护状态机
    APP_DIAG_DISCHG_VoltageProtectionHandler(&Diagnostic_Manager, currentVoltage);
    bool voltageFault = APP_DIAG_DISCHG_VoltageProtectionGetAlarmStatus(&Diagnostic_Manager);

    // 处理系统保护
    APP_DIAG_DISCHG_HandleVoltageProtection(voltageFault);
}

/****************************************放电-板载过温检测****************************************/
/**
 * @brief 初始化温度故障检测器
 * @param detector 检测器实例指针
 * @param overheatThresh 过温阈值(℃)
 * @param cooldownThresh 恢复阈值(℃)
 * @param overheatDelayMs 过温延时(ms)
 * @param cooldownDelayMs 恢复延时(ms)
 */
void APP_DIAG_DISCHG_MCUTempFault_Init(Diagnostic_Manager_t *Diagnostic,
                                    uint32_t overheatThresh, uint32_t cooldownThresh,
                                    uint32_t overheatDelayMs, uint32_t cooldownDelayMs) {
    Diagnostic->discharging.OverMCUTemp.currentState = DIAGNOSTIC_DISCHG_MCUTEMP_STATE_NORMAL;
    Diagnostic->discharging.OverMCUTemp.overheatThreshold = overheatThresh;
    Diagnostic->discharging.OverMCUTemp.cooldownThreshold = cooldownThresh;
    Diagnostic->discharging.OverMCUTemp.overheatDurationMs = overheatDelayMs;
    Diagnostic->discharging.OverMCUTemp.cooldownDurationMs = cooldownDelayMs;
    Diagnostic->discharging.OverMCUTemp.overheatCounterMs = 0;
    Diagnostic->discharging.OverMCUTemp.cooldownCounterMs = 0;
    Diagnostic->discharging.OverMCUTemp.isFaultActive = false;
}

/**
 * @brief 温度故障检测状态机处理函数（在5ms定时器中调用）
 * @param detector 检测器实例指针
 * @param currentTemp 当前温度值(℃)
 * @return true 状态发生变化, false 状态无变化
 */
bool APP_DIAG_DISCHG_MCUTempFaultProcess(Diagnostic_Manager_t *Diagnostic, uint32_t currentTemp) {
    bool stateChanged = false;
    switch (Diagnostic->discharging.OverMCUTemp.currentState) {
        case DIAGNOSTIC_DISCHG_MCUTEMP_STATE_NORMAL:
            if (currentTemp > Diagnostic->discharging.OverMCUTemp.overheatThreshold) {
                Diagnostic->discharging.OverMCUTemp.overheatCounterMs += 5; // 5ms定时周期

                if (Diagnostic->discharging.OverMCUTemp.overheatCounterMs >= Diagnostic->discharging.OverMCUTemp.overheatDurationMs) {
                    // 进入故障状态
                    Diagnostic->discharging.OverMCUTemp.currentState = DIAGNOSTIC_DISCHG_MCUTEMP_STATE_FAULT;
                    Diagnostic->discharging.OverMCUTemp.isFaultActive = true;
                    stateChanged = true;
                }
            } else {
                // 温度正常，重置过温计数器
                Diagnostic->discharging.OverMCUTemp.overheatCounterMs = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_MCUTEMP_STATE_OVERHEATING:
            // 此示例中未使用OVERHEATING状态，可预留用于更复杂的状态机
            break;

        case DIAGNOSTIC_DISCHG_MCUTEMP_STATE_FAULT:
            if (currentTemp <= Diagnostic->discharging.OverMCUTemp.cooldownThreshold) {
                // 温度降到恢复阈值以下，进入冷却状态
                Diagnostic->discharging.OverMCUTemp.currentState = DIAGNOSTIC_DISCHG_MCUTEMP_STATE_COOLING;
                Diagnostic->discharging.OverMCUTemp.cooldownCounterMs = 0;
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_DISCHG_MCUTEMP_STATE_COOLING:
            if (currentTemp > Diagnostic->discharging.OverMCUTemp.cooldownThreshold) {
                // 温度再次升高，返回故障状态
                Diagnostic->discharging.OverMCUTemp.currentState = DIAGNOSTIC_DISCHG_MCUTEMP_STATE_FAULT;
                Diagnostic->discharging.OverMCUTemp.cooldownCounterMs = 0;
                stateChanged = true;
            } else {
                Diagnostic->discharging.OverMCUTemp.cooldownCounterMs += 5; // 5ms定时周期

                if (Diagnostic->discharging.OverMCUTemp.cooldownCounterMs >= Diagnostic->discharging.OverMCUTemp.cooldownDurationMs) {
                    // 冷却完成，返回正常状态
                    Diagnostic->discharging.OverMCUTemp.currentState = DIAGNOSTIC_DISCHG_MCUTEMP_STATE_NORMAL;
                    Diagnostic->discharging.OverMCUTemp.isFaultActive = false;
                    Diagnostic->discharging.OverMCUTemp.overheatCounterMs = 0;
                    Diagnostic->discharging.OverMCUTemp.cooldownCounterMs = 0;
                    stateChanged = true;
                }
            }
            break;
    }

    return stateChanged;
}

/**
 * @brief 获取当前故障状态
 * @param detector 检测器实例指针
 * @return true 故障激活, false 故障未激活
 */
bool APP_DIAG_DISCHG_isMCUTempFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.OverMCUTemp.isFaultActive;
}

/**
 * @brief 获取当前状态机状态
 * @param detector 检测器实例指针
 * @return TempState 当前状态
 */
Diagnostic_Discharging_MCUTempState APP_DIAG_DISCHG_getMCUTempFaultState(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.OverMCUTemp.currentState;
}

/**
 * @brief 系统保护控制函数
 * @param boardFault 板子故障状态
 * @param tipFault 枪头故障状态
 */
void APP_DIAG_DISCHG_HandleOverMCUTempProtection(bool boardFault) {
    static bool lastBoardFault = false;

    event_protection_pause_discharging_data_t st_event_protection_pause_ddischarging_ata;
    event_protection_recovery_discharging_data_t st_event_protection_recovery_discharging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是接地故障引起的
    if (APP_DISCHG_IsAnyFaultPresent() && !APP_DISCHG_IsFaultPresent(E_DISCHARGING_FAULT_OVER_MCUTEMPERATURE)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (boardFault && !lastBoardFault) {
        // 执行板子过温保护动作：降低功率、关闭输出等
        if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus())
        {
            st_event_protection_pause_ddischarging_ata.type = E_DISCHARGING_PROTECTION_PAUSE_OVER_MCUTEMPERATURE;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_PAUSE, st_event_protection_pause_ddischarging_ata.type);
        }
    }

    // 检查故障恢复
    if (!boardFault && lastBoardFault) {
        if(E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus())
        {
            st_event_protection_recovery_discharging_data.type = E_DISCHARGING_PROTECTION_RECOVERY_OVER_MCUTEMPERATURE;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_RECOVERY, st_event_protection_recovery_discharging_data.type);
        }
    }

    lastBoardFault = boardFault;
}

void APP_DIAG_DISCHG_MCUTempCheck(Diagnostic_Manager_t *Diagnostic)
{
    if((E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus()) ||\
       (E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus()))
    {

    }
    else
    {
        // 初始化板子温度检测器：过温100±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
        APP_DIAG_DISCHG_MCUTempFault_Init(&Diagnostic_Manager, 9500, 7000, 5000, 5000);  //-40℃-105℃，底层计算提高50摄氏度，过温阈值95+50℃，过温恢复温度70+50摄氏度
        return;
    }

    uint32_t boardTemperature = RTE_Call_AdcM_Vlotage(ADCMCFG_MCUTemp_CH);
    // 处理板子温度检测
    bool boardStateChanged = APP_DIAG_DISCHG_MCUTempFaultProcess(&Diagnostic_Manager, boardTemperature);
    bool boardFault = APP_DIAG_DISCHG_isMCUTempFaultActive(&Diagnostic_Manager);

    // 处理系统保护（如果任一故障激活）
    APP_DIAG_DISCHG_HandleOverMCUTempProtection(boardFault);
}

/****************************************放电-NTC1过温检测****************************************/
/**
 * @brief 初始化温度故障检测器
 * @param detector 检测器实例指针
 * @param overheatThresh 过温阈值(℃)
 * @param cooldownThresh 恢复阈值(℃)
 * @param overheatDelayMs 过温延时(ms)
 * @param cooldownDelayMs 恢复延时(ms)
 */
void APP_DIAG_DISCHG_NTC1TempFault_Init(Diagnostic_Manager_t *Diagnostic,
                                    uint32_t overheatThresh, uint32_t cooldownThresh,
                                    uint32_t overheatDelayMs, uint32_t cooldownDelayMs) {
    Diagnostic->discharging.OverNTC1Temp.currentState = DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_NORMAL;
    Diagnostic->discharging.OverNTC1Temp.overheatThreshold = overheatThresh;
    Diagnostic->discharging.OverNTC1Temp.cooldownThreshold = cooldownThresh;
    Diagnostic->discharging.OverNTC1Temp.overheatDurationMs = overheatDelayMs;
    Diagnostic->discharging.OverNTC1Temp.cooldownDurationMs = cooldownDelayMs;
    Diagnostic->discharging.OverNTC1Temp.overheatCounterMs = 0;
    Diagnostic->discharging.OverNTC1Temp.cooldownCounterMs = 0;
    Diagnostic->discharging.OverNTC1Temp.isFaultActive = false;
}

/**
 * @brief 温度故障检测状态机处理函数（在5ms定时器中调用）
 * @param detector 检测器实例指针
 * @param currentTemp 当前温度值(℃)
 * @return true 状态发生变化, false 状态无变化
 */
bool APP_DIAG_DISCHG_NTC1TempFaultProcess(Diagnostic_Manager_t *Diagnostic, int currentTemp) {
    bool stateChanged = false;
    switch (Diagnostic->discharging.OverNTC1Temp.currentState) {
        case DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_NORMAL:
            if (currentTemp > Diagnostic->discharging.OverNTC1Temp.overheatThreshold) {
                Diagnostic->discharging.OverNTC1Temp.overheatCounterMs += 5; // 5ms定时周期

                if (Diagnostic->discharging.OverNTC1Temp.overheatCounterMs >= Diagnostic->discharging.OverNTC1Temp.overheatDurationMs) {
                    // 进入故障状态
                    Diagnostic->discharging.OverNTC1Temp.currentState = DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_FAULT;
                    Diagnostic->discharging.OverNTC1Temp.isFaultActive = true;
                    stateChanged = true;
                }
            } else {
                // 温度正常，重置过温计数器
                Diagnostic->discharging.OverNTC1Temp.overheatCounterMs = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_OVERHEATING:
            // 此示例中未使用OVERHEATING状态，可预留用于更复杂的状态机
            break;

        case DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_FAULT:
            if (currentTemp <= Diagnostic->discharging.OverNTC1Temp.cooldownThreshold) {
                // 温度降到恢复阈值以下，进入冷却状态
                Diagnostic->discharging.OverNTC1Temp.currentState = DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_COOLING;
                Diagnostic->discharging.OverNTC1Temp.cooldownCounterMs = 0;
                stateChanged = true;
            }
            break;

        case DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_COOLING:
            if (currentTemp > Diagnostic->discharging.OverNTC1Temp.cooldownThreshold) {
                // 温度再次升高，返回故障状态
                Diagnostic->discharging.OverNTC1Temp.currentState = DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_FAULT;
                Diagnostic->discharging.OverNTC1Temp.cooldownCounterMs = 0;
                stateChanged = true;
            } else {
                Diagnostic->discharging.OverNTC1Temp.cooldownCounterMs += 5; // 5ms定时周期

                if (Diagnostic->discharging.OverNTC1Temp.cooldownCounterMs >= Diagnostic->discharging.OverNTC1Temp.cooldownDurationMs) {
                    // 冷却完成，返回正常状态
                    Diagnostic->discharging.OverNTC1Temp.currentState = DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_NORMAL;
                    Diagnostic->discharging.OverNTC1Temp.isFaultActive = false;
                    Diagnostic->discharging.OverNTC1Temp.overheatCounterMs = 0;
                    Diagnostic->discharging.OverNTC1Temp.cooldownCounterMs = 0;
                    stateChanged = true;
                }
            }
            break;
    }

    return stateChanged;
}

/**
 * @brief 获取当前故障状态
 * @param detector 检测器实例指针
 * @return true 故障激活, false 故障未激活
 */
bool APP_DIAG_DISCHG_isNTC1TempFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.OverNTC1Temp.isFaultActive;
}

/**
 * @brief 获取当前状态机状态
 * @param detector 检测器实例指针
 * @return TempState 当前状态
 */
Diagnostic_Discharging_NTC1TempState APP_DIAG_DISCHG_getNTC1TempFaultState(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.OverNTC1Temp.currentState;
}

/**
 * @brief 系统保护控制函数
 * @param boardFault 板子故障状态
 * @param tipFault 枪头故障状态
 */
void APP_DIAG_DISCHG_HandleOverNTC1TempProtection(bool NTC1Fault) {
    static bool lastNTC1Fault = false;

    event_protection_pause_discharging_data_t st_event_protection_pause_discharging_data;
    event_protection_recovery_discharging_data_t st_event_protection_recovery_discharging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是接地故障引起的
    if (APP_DISCHG_IsAnyFaultPresent() && !APP_DISCHG_IsFaultPresent(E_DISCHARGING_FAULT_OVER_NTCTEMPERATURE)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检查状态变化
    if (NTC1Fault && !lastNTC1Fault) {
        // 执行板子过温保护动作：降低功率、关闭输出等
        if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus())
        {
            st_event_protection_pause_discharging_data.type = E_DISCHARGING_PROTECTION_PAUSE_OVER_NTCTEMPERATURE;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_PAUSE, st_event_protection_pause_discharging_data.type);
        }
    }


// 检查故障恢复
if (!NTC1Fault && lastNTC1Fault) {
    if(E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus())
    {
        st_event_protection_recovery_discharging_data.type = E_DISCHARGING_PROTECTION_RECOVERY_OVER_NTCTEMPERATURE;
        APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_RECOVERY, st_event_protection_recovery_discharging_data.type);
    }
}
lastNTC1Fault = NTC1Fault;
}

void APP_DIAG_DISCHG_NTC1TempCheck(Diagnostic_Manager_t *Diagnostic)
{
    if((E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus()) ||\
       (E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus()))
    {

    }
    else
    {
        // 初始化板子温度检测器：过温105±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
        APP_DIAG_DISCHG_NTC1TempFault_Init(&Diagnostic_Manager, 100, 70, 5000, 5000);
        return;
    }

    int NTC1Temperature = (int)(RTE_Call_AdcM_Vlotage(ADCMCFG_NTC1Temp_CH) - 25);
    // 处理板子温度检测
    bool NTC1StateChanged = APP_DIAG_DISCHG_NTC1TempFaultProcess(&Diagnostic_Manager, NTC1Temperature);
    bool NTC1Fault = APP_DIAG_DISCHG_isNTC1TempFaultActive(&Diagnostic_Manager);

    // 处理系统保护（如果任一故障激活）
    APP_DIAG_DISCHG_HandleOverNTC1TempProtection(NTC1Fault);
}

/****************************************放电-过流****************************************/
/**
 * @brief 初始化过流保护器
 * @param protector 保护器实例指针
 */
void APP_DIAG_DISCHG_CurProtector_Init(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_NORMAL;

    // 初始化配置参数
    Diagnostic->discharging.Overcurrent.config.level1Threshold = 35000;    // 一段固定35A
    Diagnostic->discharging.Overcurrent.config.level2Threshold = 40000;    // 二段固定40A
    Diagnostic->discharging.Overcurrent.config.level1DurationMs = 5000;    // 5秒
    Diagnostic->discharging.Overcurrent.config.level2DurationMs = 10;      // 10ms
    Diagnostic->discharging.Overcurrent.config.recoveryDelayMs = 30000;    // 30秒恢复时间
    Diagnostic->discharging.Overcurrent.config.recoveryCheckMs = 2000;     // 2秒恢复检测时间
    Diagnostic->discharging.Overcurrent.config.maxLevel1Faults = 3;        // 最大3次

    // 初始化恢复检测器
    Diagnostic->discharging.Overcurrent.recovery.state = DIAGNOSTIC_DISCHG_OC_RECOVERY_IDLE;
    Diagnostic->discharging.Overcurrent.recovery.checkCounterMs = 0;
    Diagnostic->discharging.Overcurrent.recovery.countdownCounterMs = 0;
    Diagnostic->discharging.Overcurrent.recovery.conditionMet = false;
    Diagnostic->discharging.Overcurrent.recovery.currentThreshold = (float)ADCM_t.cali.current.cail_b/10;

    // 初始化运行数据
    Diagnostic->discharging.Overcurrent.level1CounterMs = 0;
    Diagnostic->discharging.Overcurrent.level2CounterMs = 0;
    Diagnostic->discharging.Overcurrent.level1FaultCount = 0;
    Diagnostic->discharging.Overcurrent.isFaultActive = false;
    Diagnostic->discharging.Overcurrent.isChargingLocked = false;
    Diagnostic->discharging.Overcurrent.currentCurrent = 0.0f;
}

void APP_DIAG_DISCHG_CurProtector_Init2(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_NORMAL;

    // 初始化配置参数
    Diagnostic->discharging.Overcurrent.config.level1Threshold = 35000;    // 一段固定35A
    Diagnostic->discharging.Overcurrent.config.level2Threshold = 40000;    // 二段固定40A
    Diagnostic->discharging.Overcurrent.config.level1DurationMs = 5000;    // 5秒
    Diagnostic->discharging.Overcurrent.config.level2DurationMs = 10;      // 10ms
    Diagnostic->discharging.Overcurrent.config.recoveryDelayMs = 30000;    // 30秒恢复时间
    Diagnostic->discharging.Overcurrent.config.recoveryCheckMs = 2000;     // 2秒恢复检测时间
    Diagnostic->discharging.Overcurrent.config.maxLevel1Faults = 3;        // 最大3次

    // 初始化恢复检测器
    Diagnostic->discharging.Overcurrent.recovery.state = DIAGNOSTIC_DISCHG_OC_RECOVERY_IDLE;
    Diagnostic->discharging.Overcurrent.recovery.checkCounterMs = 0;
    Diagnostic->discharging.Overcurrent.recovery.countdownCounterMs = 0;
    Diagnostic->discharging.Overcurrent.recovery.conditionMet = false;
    Diagnostic->discharging.Overcurrent.recovery.currentThreshold = (float)ADCM_t.cali.current.cail_b/10;

    // 初始化运行数据
    Diagnostic->discharging.Overcurrent.level1CounterMs = 0;
    Diagnostic->discharging.Overcurrent.level2CounterMs = 0;
//    Diagnostic->discharging.Overcurrent.level1FaultCount = 0;
    Diagnostic->discharging.Overcurrent.isFaultActive = false;
    Diagnostic->discharging.Overcurrent.isChargingLocked = false;
    Diagnostic->discharging.Overcurrent.currentCurrent = 0.0f;
}

/**
 * @brief 重置恢复检测器
 * @param protector 保护器实例指针
 */
static void APP_DIAG_DISCHG_CurresetRecovery(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->discharging.Overcurrent.recovery.state = DIAGNOSTIC_DISCHG_OC_RECOVERY_IDLE;
    Diagnostic->discharging.Overcurrent.recovery.checkCounterMs = 0;
    Diagnostic->discharging.Overcurrent.recovery.countdownCounterMs = 0;
    Diagnostic->discharging.Overcurrent.recovery.conditionMet = false;
}

/**
 * @brief 过流保护处理函数（5ms定时器调用）
 * @param protector 保护器实例指针
 * @param measuredCurrent 测量电流值(A)
 */
void APP_DIAG_DISCHG_CurProcess(Diagnostic_Manager_t *Diagnostic, uint32_t measuredCurrent) {
    Diagnostic->discharging.Overcurrent.currentCurrent = measuredCurrent;

    switch (Diagnostic->discharging.Overcurrent.currentState) {
        case DIAGNOSTIC_DISCHG_OC_STATE_NORMAL:
            APP_DIAG_DISCHG_CurresetRecovery(Diagnostic);

            if(measuredCurrent > Diagnostic->discharging.Overcurrent.config.level2Threshold) { //大于2段过流值
                Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_LEVEL2_FAULT;
                Diagnostic->discharging.Overcurrent.level2CounterMs = 5;
            }
            else if (measuredCurrent > Diagnostic->discharging.Overcurrent.config.level1Threshold) { //大于1段过流值
                Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_LEVEL1_DETECTING;
                Diagnostic->discharging.Overcurrent.level1CounterMs = 5;
            }
            break;

        case DIAGNOSTIC_DISCHG_OC_STATE_LEVEL1_DETECTING:
            if(measuredCurrent > Diagnostic->discharging.Overcurrent.config.level2Threshold) { //大于2段过流值
                Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_LEVEL2_FAULT;
                Diagnostic->discharging.Overcurrent.level2CounterMs = 5;
                Diagnostic->discharging.Overcurrent.level1CounterMs = 0;
            }
            else if (measuredCurrent > Diagnostic->discharging.Overcurrent.config.level1Threshold) { //大于段过流值
                Diagnostic->discharging.Overcurrent.level1CounterMs += 5;
                if (Diagnostic->discharging.Overcurrent.level1CounterMs >= Diagnostic->discharging.Overcurrent.config.level1DurationMs) { //大于1段过流值次数+1
                    Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_LEVEL1_FAULT;
                    Diagnostic->discharging.Overcurrent.isFaultActive = true;
                    Diagnostic->discharging.Overcurrent.level1FaultCount++;
                    APP_DIAG_DISCHG_CurresetRecovery(Diagnostic);
                }
            }
            else {
                Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_NORMAL;
                Diagnostic->discharging.Overcurrent.level1CounterMs = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_OC_STATE_LEVEL1_FAULT:
            // 恢复条件检测
            if(Diagnostic->discharging.Overcurrent.level1FaultCount >= Diagnostic->discharging.Overcurrent.config.maxLevel1Faults) { //大于1段过流值3次不恢复，进入锁死状态
                Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_LOCKED;
                Diagnostic->discharging.Overcurrent.isChargingLocked = true;
            }

            if (measuredCurrent <= Diagnostic->discharging.Overcurrent.recovery.currentThreshold) {  //电流小于0A，开始恢复
                if (Diagnostic->discharging.Overcurrent.recovery.state == DIAGNOSTIC_DISCHG_OC_RECOVERY_IDLE) {
                    Diagnostic->discharging.Overcurrent.recovery.state = DIAGNOSTIC_DISCHG_OC_RECOVERY_CHECKING;
                }

                Diagnostic->discharging.Overcurrent.recovery.checkCounterMs += 5;

                if (Diagnostic->discharging.Overcurrent.recovery.checkCounterMs >= Diagnostic->discharging.Overcurrent.config.recoveryCheckMs) { //检测到0A持续2s后开始30s倒计时
                    Diagnostic->discharging.Overcurrent.recovery.state = DIAGNOSTIC_DISCHG_OC_RECOVERY_CONDITION_MET;
                    Diagnostic->discharging.Overcurrent.recovery.conditionMet = true;
                }
            }
            else {
                APP_DIAG_DISCHG_CurresetRecovery(Diagnostic);
                Diagnostic->discharging.Overcurrent.recovery.countdownCounterMs = 0;
            }

            // 恢复倒计时
            if (Diagnostic->discharging.Overcurrent.recovery.conditionMet) {
                Diagnostic->discharging.Overcurrent.recovery.state = DIAGNOSTIC_DISCHG_OC_RECOVERY_COUNTDOWN;
                Diagnostic->discharging.Overcurrent.recovery.countdownCounterMs += 5;

                if (Diagnostic->discharging.Overcurrent.recovery.countdownCounterMs >= Diagnostic->discharging.Overcurrent.config.recoveryDelayMs) {
                    Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_NORMAL;
                    Diagnostic->discharging.Overcurrent.isFaultActive = false;
                    APP_DIAG_DISCHG_CurresetRecovery(Diagnostic);
                }
            }
            break;

        case DIAGNOSTIC_DISCHG_OC_STATE_LEVEL2_FAULT:
            APP_DIAG_DISCHG_CurresetRecovery(Diagnostic);
            if(measuredCurrent > Diagnostic->discharging.Overcurrent.config.level2Threshold) {
                Diagnostic->discharging.Overcurrent.level2CounterMs += 5;

                if (Diagnostic->discharging.Overcurrent.level2CounterMs >= Diagnostic->discharging.Overcurrent.config.level2DurationMs) {
                    Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_LOCKED;
                    Diagnostic->discharging.Overcurrent.isFaultActive = true;
                    Diagnostic->discharging.Overcurrent.isChargingLocked = true;
                }
            }
            else{
                Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_NORMAL; //持续3s判定为二段过流，3s内检测低于2段过流值重新判定
                Diagnostic->discharging.Overcurrent.level2CounterMs = 0;
            }
            break;

        case DIAGNOSTIC_DISCHG_OC_STATE_LOCKED:
            // 一段过流拔枪可恢复，二段、三段过流重新上电恢复
            break;
    }
}

// 其他函数实现
bool APP_DIAG_DISCHG_CurIsFaultActive(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.Overcurrent.isFaultActive;
}

bool APP_DIAG_DISCHG_CurIsLocked(const Diagnostic_Manager_t *Diagnostic) {
    return Diagnostic->discharging.Overcurrent.isChargingLocked;
}

void APP_DIAG_DISCHG_CurReset(Diagnostic_Manager_t *Diagnostic) {
    Diagnostic->discharging.Overcurrent.currentState = DIAGNOSTIC_DISCHG_OC_STATE_NORMAL;
    Diagnostic->discharging.Overcurrent.level1CounterMs = 0;
    Diagnostic->discharging.Overcurrent.level2CounterMs = 0;
    Diagnostic->discharging.Overcurrent.level1FaultCount = 0;
    Diagnostic->discharging.Overcurrent.isFaultActive = false;
    Diagnostic->discharging.Overcurrent.isChargingLocked = false;
    APP_DIAG_DISCHG_CurresetRecovery(Diagnostic);
}

//void APP_Discharg_DIAG_OCReset(void)
//{
//    APP_DIAG_DISCHG_CurReset(&Diagnostic_Manager);
//}

void APP_DIAG_DISCHG_HandleOverCurrentProtection(bool currentFault, bool currentLocked)
{
    static bool lastFaultState = false;
    static bool lastLockedState = false;

    event_fault_discharging_data_t st_event_fault_discharging_data;
    event_protection_pause_discharging_data_t st_event_protection_pause_discharging_data;
    event_protection_recovery_discharging_data_t st_event_protection_recovery_discharging_data;

    // 前置检查：如果系统有其他更高优先级的故障，且不是过流故障引起的
    if (APP_DISCHG_IsAnyFaultPresent() && !APP_DISCHG_IsFaultPresent(E_DISCHARGING_FAULT_OVER_CURRENT)) {
        return; // 系统被其他故障锁定，不处理接地故障
    }

    // 检测状态变化
    bool faultChanged = (currentFault != lastFaultState);
    bool lockedChanged = (currentLocked != lastLockedState);

    if (!faultChanged && !lockedChanged) {
        return; // 状态无变化，不处理事件
    }

    // 判断事件类型
    if (currentLocked) {
        // 二段过流锁死或者是一段过流超3次
        if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus())
        {
            st_event_fault_discharging_data.type = E_DISCHARGING_FAULT_OVER_CURRENT;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_FAULT,st_event_fault_discharging_data.type);
        }
    }
    else if (currentFault) {
        // 一段过流保护
        if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus())
        {
            st_event_protection_pause_discharging_data.type = E_DISCHARGING_PROTECTION_PAUSE_CURRENT;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_PAUSE, st_event_protection_pause_discharging_data.type);
        }
    }
    else if (lastFaultState && !currentFault) {
        // 一段过流恢复
        if(E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus())
        {
            st_event_protection_recovery_discharging_data.type = E_DISCHARGING_PROTECTION_RECOVERY_CURRENT;
            APP_DISCHG_EventPub(E_EVENT_DISCHG_PROTECTION_RECOVERY, st_event_protection_recovery_discharging_data.type);
        }
    }

    lastFaultState = currentFault;
    lastLockedState = currentLocked;
}

void APP_DIAG_DISCHG_OverCurrentCheck(Diagnostic_Manager_t *Diagnostic)
{
//过流保护在充电中、保护暂停状态下保持检测
    if(E_STATE_DISCHG_DISCHARGING == APP_DISCHG_GetChargeStatus() ||\
       E_STATE_DISCHG_PPAUSE == APP_DISCHG_GetChargeStatus())
    {

    }
    else
    {
        // 初始化过流保护器
        APP_DIAG_DISCHG_CurProtector_Init2(&Diagnostic_Manager);
        return;
    }

    uint32_t current = RTE_Call_AdcM_Vlotage(ADCMCFG_Currnet_CH);
    APP_DIAG_DISCHG_CurProcess(&Diagnostic_Manager, current);
    bool currentFault = APP_DIAG_DISCHG_CurIsFaultActive(&Diagnostic_Manager);
    bool currentLocked = APP_DIAG_DISCHG_CurIsLocked(&Diagnostic_Manager);

    APP_DIAG_DISCHG_HandleOverCurrentProtection(currentFault, currentLocked);
}

/****************************************充电诊断初始化****************************************/
void APP_DIAG_CHG_Init(void)
{
    // 初始化漏电检测数据
    APP_DIAG_CHG_LeakageCheck_Init(&Diagnostic_Manager);

    // 初始化短路检测数据
    APP_DIAG_CHG_ShortCircuit_Init(&Diagnostic_Manager);

    // 初始化板子温度检测器：过温100±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
    APP_DIAG_CHG_MCUTempFault_Init(&Diagnostic_Manager, 9500, 7000, 5000, 5000); //-40℃-105℃，底层计算提高50摄氏度，过温阈值95+50℃，过温恢复温度70+50摄氏度

    // 初始NTC1温度检测器：过温100±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
    APP_DIAG_CHG_MCUTempFault_Init(&Diagnostic_Manager, 100, 70, 5000, 5000); //-40℃-105℃，底层计算提高50摄氏度，过温阈值95+50℃，过温恢复温度70+50摄氏度


	
    // 初始化继电器故障检测器：故障确认50ms，故障恢复1000ms
    APP_DIAG_CHG_RelayFault_Init(&Diagnostic_Manager, 50, 1000);

    // 初始化过流保护器
    APP_DIAG_CHG_CurProtector_Init(&Diagnostic_Manager);

    //过欠压
    APP_DIAG_CHG_voltageProtector_Init(&Diagnostic_Manager);

    //接地异常
    APP_DIAG_CHG_GroundFault_Init(&Diagnostic_Manager);
}

//1ms周期运行
void APP_DIAG_CHG_Update(void) {
    APP_DIAG_CHG_LeakageCheck_Start(&Diagnostic_Manager);  //漏电
    APP_DIAG_CHG_ShortCircuit_Trigger(&Diagnostic_Manager); //短路
    APP_DIAG_CHG_ShortCircuit_Update(&Diagnostic_Manager);  //短路
    APP_DIAG_CHG_MCUTempCheck(&Diagnostic_Manager);         //板载过温
    APP_DIAG_CHG_RelayCheck(&Diagnostic_Manager);           //继电器黏连
    APP_DIAG_CHG_OverCurrentCheck(&Diagnostic_Manager);     //过流
    APP_DIAG_CHG_VoltageCheck(&Diagnostic_Manager);         //过欠压
    APP_DIAG_CHG_GroundCheck(&Diagnostic_Manager);          //接地
}

/****************************************放电诊断部分初始化****************************************/
void APP_DIAG_DISCHG_Init(void)
{
    // 初始化漏电检测数据
    APP_DIAG_DISCHG_LeakageCheck_Init(&Diagnostic_Manager);

    // 初始化板子温度检测器：过温105±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
    APP_DIAG_DISCHG_MCUTempFault_Init(&Diagnostic_Manager, 95000, 7000, 5000, 5000); //-40℃-105℃，底层计算提高50摄氏度，过温阈值95+50℃，过温恢复温度70+50摄氏度

    // 初始化NTC1温度检测器：过温105±5℃，恢复65±5℃，过温延时5000ms，恢复延时5000ms
    APP_DIAG_DISCHG_NTC1TempFault_Init(&Diagnostic_Manager, 100, 70, 5000, 5000);

    // 初始化过流保护器
    APP_DIAG_DISCHG_CurProtector_Init(&Diagnostic_Manager);

    //过欠压
    APP_DIAG_DISCHG_VoltageProtection_Init(&Diagnostic_Manager);
}

void APP_DIAG_DISCHG_Update(void) {
    APP_DIAG_DISCHG_LeakageCheck_Start(&Diagnostic_Manager);//漏电
    APP_DIAG_DISCHG_VoltageCheck(&Diagnostic_Manager);     //过欠压
    APP_DIAG_DISCHG_OverCurrentCheck(&Diagnostic_Manager); //过流
    APP_DIAG_DISCHG_MCUTempCheck(&Diagnostic_Manager);     //板载过温
    //APP_DIAG_DISCHG_NTC1TempCheck(&Diagnostic_Manager);    //插排过温
}

void APP_Diag_ChargeManageMain_1ms(void)    //漏电1ms任务
{
    APP_DIAG_CHG_LeakageCheck(&Diagnostic_Manager);
    APP_DIAG_DISCHG_LeakageCheck(&Diagnostic_Manager);
}
