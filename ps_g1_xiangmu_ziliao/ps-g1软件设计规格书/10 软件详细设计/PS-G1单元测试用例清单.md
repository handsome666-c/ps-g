# PS-G1 软件详细设计 — 单元测试用例清单

> 基于PS-G1软件详细设计规格书（ASW 10个模块 + BSW 4个模块）整理  
> 编制日期：2026-04-24  
> 版本：A0

---

## 目录

- [1 ASW层 — CP模块](#1-asw层--cp模块)
- [2 ASW层 — Charging模块](#2-asw层--charging模块)
- [3 ASW层 — Diag模块](#3-asw层--diag模块)
- [4 ASW层 — Discharging模块](#4-asw层--discharging模块)
- [5 ASW层 — LED模块](#5-asw层--led模块)
- [6 ASW层 — Factory模块](#6-asw层--factory模块)
- [7 ASW层 — Log模块](#7-asw层--log模块)
- [8 ASW层 — Calibration模块](#8-asw层--calibration模块)
- [9 ASW层 — 电压测量模块](#9-asw层--电压测量模块)
- [10 ASW层 — 电流测量模块](#10-asw层--电流测量模块)
- [11 BSW层 — AdcF模块](#11-bsw层--adcf模块)
- [12 BSW层 — AdcM模块](#12-bsw层--adcm模块)
- [13 BSW层 — EcuM模块](#13-bsw层--ecum模块)
- [14 BSW层 — WdgM模块](#14-bsw层--wdgm模块)

---

## 1 ASW层 — CP模块

**核心职责**：依据GB/T 18487.1标准，通过检测CP信号线上的电压状态和PWM信号占空比，管理与电动汽车充电连接过程。

### 1.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CP-001 | APP_CP_Task | CP任务正常调度 | 1. 初始化CP状态机 2. 调用APP_CP_Task 3. 验证内部依次调用电压滤波→状态检测→状态机处理→事件处理 | 任务函数正常执行完毕，无异常返回 |
| CP-002 | APP_CP_Task | CP任务重复调用稳定性 | 1. 连续调用APP_CP_Task 1000次 2. 检查每次执行无异常 | 1000次调用均正常完成 |
| CP-003 | APP_CP_SetState | 设置PWM激活状态 | 1. 调用APP_CP_SetState(PWM_ACTIVE) 2. 读取内部PWM输出状态 | PWM输出状态置为激活 |
| CP-004 | APP_CP_SetState | 设置PWM非激活状态 | 1. 调用APP_CP_SetState(PWM_INACTIVE) 2. 读取内部PWM输出状态 | PWM输出状态置为非激活 |
| CP-005 | APP_CP_SetState | 边界值测试-pwm_active=0 | 1. 调用APP_CP_SetState(0) 2. 检查PWM输出状态 | PWM输出状态为非激活 |
| CP-006 | APP_CP_SetState | 边界值测试-pwm_active=255 | 1. 调用APP_CP_SetState(255) 2. 检查PWM输出状态 | PWM输出状态为激活 |
| CP-007 | APP_CP_GetCurrentStatus | 初始状态获取 | 1. 初始化CP模块 2. 调用APP_CP_GetCurrentStatus | 返回初始状态（如STATE_A或默认待机状态） |
| CP-008 | APP_CP_GetCurrentStatus | 状态变化后获取 | 1. 触发状态机转换（如插枪事件） 2. 调用APP_CP_GetCurrentStatus | 返回转换后的当前状态 |
| CP-009 | APP_CP_DetectCPState | 12V电压检测 | 1. 输入voltage=1200u（12V对应值），pwm_active=1 2. 调用APP_CP_DetectCPState | 返回CP_VOLTAGE_12V状态 |
| CP-010 | APP_CP_DetectCPState | 9V电压检测 | 1. 输入voltage=900u（9V对应值），pwm_active=1 2. 调用APP_CP_DetectCPState | 返回CP_VOLTAGE_9V状态 |
| CP-011 | APP_CP_DetectCPState | 6V电压检测 | 1. 输入voltage=600u（6V对应值），pwm_active=1 2. 调用APP_CP_DetectCPState | 返回CP_VOLTAGE_6V状态 |
| CP-012 | APP_CP_DetectCPState | 异常高电压检测 | 1. 输入voltage=5000u（超12V范围），pwm_active=1 2. 调用APP_CP_DetectCPState | 返回异常状态 |
| CP-013 | APP_CP_DetectCPState | 异常低电压检测 | 1. 输入voltage=50u（低于6V范围），pwm_active=1 2. 调用APP_CP_DetectCPState | 返回异常状态 |
| CP-014 | APP_CP_DetectCPState | 12V边界值-上限 | 1. 输入voltage=1320u（12V+1.2V上限），pwm_active=1 2. 调用 | 返回CP_VOLTAGE_12V状态 |
| CP-015 | APP_CP_DetectCPState | 12V边界值-下限 | 1. 输入voltage=1080u（12V-1.2V下限），pwm_active=1 2. 调用 | 返回CP_VOLTAGE_12V状态 |
| CP-016 | APP_CP_DetectCPState | PWM未激活时检测 | 1. 输入voltage=900u，pwm_active=0 2. 调用APP_CP_DetectCPState | 按设计返回对应非PWM状态 |
| CP-017 | APP_CP_DetectCPState | 0电压输入 | 1. 输入voltage=0，pwm_active=1 2. 调用APP_CP_DetectCPState | 返回异常或最低电压状态 |

### 1.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CP-018 | is_in_abnormal_zone | 正常12V区域内电压 | 1. 输入voltage=1200u 2. 调用is_in_abnormal_zone | 返回false（非异常区域） |
| CP-019 | is_in_abnormal_zone | 正常9V区域内电压 | 1. 输入voltage=900u 2. 调用is_in_abnormal_zone | 返回false |
| CP-020 | is_in_abnormal_zone | 正常6V区域内电压 | 1. 输入voltage=600u 2. 调用is_in_abnormal_zone | 返回false |
| CP-021 | is_in_abnormal_zone | 异常区域-两区间之间 | 1. 输入voltage=1050u（9V~12V之间的间隙） 2. 调用 | 返回true（在异常区域） |
| CP-022 | is_in_abnormal_zone | 异常区域-极低电压 | 1. 输入voltage=100u 2. 调用is_in_abnormal_zone | 返回true |
| CP-023 | is_in_abnormal_zone | 异常区域-极高电压 | 1. 输入voltage=2000u 2. 调用is_in_abnormal_zone | 返回true |
| CP-024 | APP_CP_GetPwmStatus | PWM正在输出 | 1. 设置PWM输出为激活 2. 调用APP_CP_GetPwmStatus | 返回PWM激活状态 |
| CP-025 | APP_CP_GetPwmStatus | PWM未输出 | 1. 设置PWM输出为非激活 2. 调用APP_CP_GetPwmStatus | 返回PWM非激活状态 |
| CP-026 | APP_CP_ProcessStateMachine | 状态A→状态B转换 | 1. 初始化状态机为STATE_A 2. 输入cp_state=CP_VOLTAGE_9V 3. 调用ProcessStateMachine | 返回对应事件，状态机转换正确 |
| CP-027 | APP_CP_ProcessStateMachine | 状态无变化 | 1. 初始化状态机 2. 输入当前cp_state与上一致 3. 调用ProcessStateMachine | 返回无事件或保持事件 |
| CP-028 | APP_CP_HandleChargerEvent | 处理插枪事件 | 1. 输入event=EVENT_PLUGIN，cp_state=CP_VOLTAGE_9V 2. 调用HandleChargerEvent | 正确处理事件，调用对应状态转换 |
| CP-029 | APP_CP_HandleChargerEvent | 处理故障事件 | 1. 输入event=EVENT_FAULT 2. 调用HandleChargerEvent | 触发故障处理流程 |
| CP-030 | CP_Voltage_Filter | 正常电压滤波 | 1. 模拟多次ADC采样值 2. 调用CP_Voltage_Filter 3. 验证滤波结果 | 返回滤波后的稳定电压值 |
| CP-031 | CP_Voltage_Filter | 突变电压滤波 | 1. 模拟正常采样中突然出现异常值 2. 调用CP_Voltage_Filter | 滤波后应抑制突变，输出平滑值 |

---

## 2 ASW层 — Charging模块

**核心职责**：通过状态机执行充电输入和输出命令，为充电部分软件应用功能核心。

### 2.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CHG-001 | APP_CHG_EventPubTopic | 发布插枪事件 | 1. 调用APP_CHG_EventPubTopic(EVENT_PLUGIN) 2. 检查事件队列 | 事件成功入队，返回成功 |
| CHG-002 | APP_CHG_EventPubTopic | 发布拔枪事件 | 1. 调用APP_CHG_EventPubTopic(EVENT_UNPLUG) 2. 检查事件队列 | 事件成功入队 |
| CHG-003 | APP_CHG_EventPubTopic | 发布自检成功事件 | 1. 调用APP_CHG_EventPubTopic(EVENT_SELFCHECK_OK) 2. 检查事件队列 | 事件成功入队 |
| CHG-004 | APP_CHG_EventPubTopic | 发布预检成功事件 | 1. 调用APP_CHG_EventPubTopic(EVENT_PRECHECK_OK) 2. 检查事件队列 | 事件成功入队 |
| CHG-005 | APP_CHG_EventPub | 发布告警事件带故障码 | 1. 调用APP_CHG_EventPub(EVENT_ALARM, 0x01) 2. 检查事件与数据 | 事件及故障码成功入队 |
| CHG-006 | APP_CHG_EventPub | 发布保护暂停事件 | 1. 调用APP_CHG_EventPub(EVENT_PROTECT_PAUSE, faultCode) 2. 检查 | 事件及数据成功入队 |
| CHG-007 | APP_CHG_EventPub | 发布故障事件 | 1. 调用APP_CHG_EventPub(EVENT_FAULT, faultCode) 2. 检查 | 故障事件及码成功入队 |
| CHG-008 | APP_CHG_StateMachineDispatch | 空闲状态下调度 | 1. 初始化充电状态机为IDLE 2. 调用StateMachineDispatch 3. 无事件时 | 返回E_ChargeStateMachineRun_OK，状态不变 |
| CHG-009 | APP_CHG_StateMachineDispatch | 有事件时调度 | 1. 先发布插枪事件 2. 调用StateMachineDispatch | 状态机正确响应事件，状态转换 |
| CHG-010 | APP_CHG_StateMachineDispatch | 连续调度稳定性 | 1. 连续调用StateMachineDispatch 1000次 2. 检查无异常 | 每次均返回E_ChargeStateMachineRun_OK |
| CHG-011 | APP_CHG_StateMachineInit | 初始化验证 | 1. 调用APP_CHG_StateMachineInit 2. 读取上一次状态和当前状态 | 上一次状态与当前状态均初始化为默认值（IDLE） |
| CHG-012 | APP_CHG_StateMachineInit | 重复初始化 | 1. 调用APP_CHG_StateMachineInit两次 2. 检查状态 | 第二次初始化后状态正确重置 |
| CHG-013 | APP_CHG_GetChargeStatus | 初始状态获取 | 1. 初始化充电模块 2. 调用APP_CHG_GetChargeStatus | 返回IDLE或默认初始状态 |
| CHG-014 | APP_CHG_GetChargeStatus | 充电中状态获取 | 1. 触发充电流程使状态进入CHARGING 2. 调用GetChargeStatus | 返回CHARGING状态 |
| CHG-015 | APP_CHG_GetStatusLED | 空闲状态LED | 1. 充电状态为IDLE 2. 调用APP_CHG_GetStatusLED | 返回对应空闲LED状态值 |
| CHG-016 | APP_CHG_GetStatusLED | 充电中LED | 1. 充电状态为CHARGING 2. 调用APP_CHG_GetStatusLED | 返回对应充电LED状态值 |
| CHG-017 | APP_CHG_GetStatusLED | 故障状态LED | 1. 充电状态为FAULT 2. 调用APP_CHG_GetStatusLED | 返回对应故障LED状态值 |
| CHG-018 | APP_CHG_GetFault | 初始无故障 | 1. 初始化充电模块 2. 调用APP_CHG_GetFault | 返回0（无故障码） |
| CHG-019 | APP_CHG_GetFault | 有故障码 | 1. 触发过流故障 2. 调用APP_CHG_GetFault | 返回对应故障码 |
| CHG-020 | APP_CHG_IsFaultPresent | 特定故障存在 | 1. 触发过流故障 2. 调用APP_CHG_IsFaultPresent(OVER_CURRENT) | 返回true |
| CHG-021 | APP_CHG_IsFaultPresent | 特定故障不存在 | 1. 触发过流故障 2. 调用APP_CHG_IsFaultPresent(OVER_VOLTAGE) | 返回false |
| CHG-022 | APP_CHG_IsFaultPresent | 无任何故障时查询 | 1. 初始化后无故障 2. 调用APP_CHG_IsFaultPresent(任意类型) | 返回false |
| CHG-023 | APP_CHG_IsAnyFaultPresent | 无故障 | 1. 初始化充电模块 2. 调用APP_CHG_IsAnyFaultPresent | 返回false |
| CHG-024 | APP_CHG_IsAnyFaultPresent | 有故障 | 1. 触发任意故障 2. 调用APP_CHG_IsAnyFaultPresent | 返回true |
| CHG-025 | APP_CHG_GetDiodeStatus | 二极管正常 | 1. 设置二极管检测结果为正常 2. 调用APP_CHG_GetDiodeStatus | 返回DIODE_OK |
| CHG-026 | APP_CHG_GetDiodeStatus | 二极管异常 | 1. 设置二极管检测结果为异常 2. 调用APP_CHG_GetDiodeStatus | 返回DIODE_ERROR |

---

## 3 ASW层 — Diag模块

**核心职责**：周期诊断产品功能性是否正常运行，实时监控并上报。包含充电诊断（CHG）和放电诊断（DISCHG）两大部分。

### 3.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| DIAG-001 | APP_DIAG_CHG_CurRset | 充电电流参数复位 | 1. 设置电流检测各项参数为非零值 2. 调用APP_DIAG_CHG_CurRset 3. 检查各参数 | 各参数归零/初始值 |
| DIAG-002 | APP_DIAG_CHG_CurRset | 空指针保护 | 1. 传入NULL指针 2. 调用APP_DIAG_CHG_CurRset | 函数不崩溃（应有空指针保护） |
| DIAG-003 | APP_DIAG_DISCHG_CurRset | 放电电流参数复位 | 1. 设置放电电流检测参数为非零值 2. 调用APP_DIAG_DISCHG_CurRset 3. 检查各参数 | 各参数归零/初始值 |
| DIAG-004 | APP_DIAG_CHG_Init | 充电Diag初始化 | 1. 调用APP_DIAG_CHG_Init 2. 检查内部各诊断子模块初始化状态 | 所有充电诊断子模块参数初始化为默认值 |
| DIAG-005 | APP_DIAG_CHG_Init | 重复初始化 | 1. 调用APP_DIAG_CHG_Init两次 2. 检查状态 | 第二次初始化正确重置所有参数 |
| DIAG-006 | APP_DIAG_CHG_Update | 充电Diag正常更新 | 1. 初始化Diag 2. 调用APP_DIAG_CHG_Update 3. 检查各诊断结果更新 | 各诊断子模块数据正确更新 |
| DIAG-007 | APP_DIAG_CHG_Update | 故障条件下更新 | 1. 模拟过流条件 2. 调用APP_DIAG_CHG_Update 3. 检查过流故障标志 | 过流故障标志置位 |
| DIAG-008 | APP_DIAG_DISCHG_Init | 放电Diag初始化 | 1. 调用APP_DIAG_DISCHG_Init 2. 检查内部各诊断子模块初始化 | 所有放电诊断子模块参数初始化为默认值 |
| DIAG-009 | APP_DIAG_DISCHG_Update | 放电Diag正常更新 | 1. 初始化Diag 2. 调用APP_DIAG_DISCHG_Update 3. 检查诊断结果 | 各诊断子模块数据正确更新 |
| DIAG-010 | APP_DIAG_ChargeManageMain_1ms | 漏电1ms任务正常执行 | 1. 调用APP_DIAG_ChargeManageMain_1ms 2. 检查漏电检测计时器递增 | 计时器正确递增，漏电检测逻辑正常执行 |
| DIAG-011 | APP_DIAG_ChargeManageMain_1ms | 漏电1ms任务周期执行 | 1. 连续调用200次（模拟200ms） 2. 检查漏电检测是否触发判定 | 达到阈值后正确触发漏电判定 |

### 3.2 内部函数 — 充电诊断

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| DIAG-012 | APP_DIAG_CHG_LeakageCheck_Init | 漏电检测初始化 | 1. 调用初始化函数 2. 检查漏电检测参数 | 参数清零，状态机初始化 |
| DIAG-013 | APP_DIAG_CHG_ShortCircuit_Init | 短路检测初始化 | 1. 调用初始化函数 2. 检查短路检测参数 | 参数清零，检测标志初始化 |
| DIAG-014 | APP_DIAG_CHG_ShortCircuit_Update | 短路正常更新 | 1. 模拟正常ADC值 2. 调用更新函数 | 无短路故障触发 |
| DIAG-015 | APP_DIAG_CHG_ShortCircuit_Check | 短路检测-正常 | 1. 输入正常短路检测采样值 2. 调用检测函数 | 无故障 |
| DIAG-016 | APP_DIAG_CHG_ShortCircuit_Check | 短路检测-短路发生 | 1. 输入异常短路检测采样值 2. 持续调用达到持续时间阈值 | 短路故障标志置位 |
| DIAG-017 | APP_DIAG_CHG_ShortCircuit_GetFaultStatus | 获取短路故障状态-无故障 | 1. 无短路条件 2. 调用GetFaultStatus | 返回false |
| DIAG-018 | APP_DIAG_CHG_ShortCircuit_GetFaultStatus | 获取短路故障状态-有故障 | 1. 触发短路故障 2. 调用GetFaultStatus | 返回true |
| DIAG-019 | APP_DIAG_CHG_MCU_TempFault_Init | MCU温度初始化 | 1. 传入过热阈值100、恢复阈值80、延时200ms、恢复延时1000ms 2. 调用初始化 | 各阈值和延时参数正确设置 |
| DIAG-020 | APP_DIAG_CHG_MCUTempFaultProcess | MCU正常温度 | 1. 输入currentTemp=50（正常） 2. 调用FaultProcess | 返回false，无故障 |
| DIAG-021 | APP_DIAG_CHG_MCUTempFaultProcess | MCU过温触发 | 1. 输入currentTemp=105（超过阈值） 2. 持续调用超过延时时间 3. 检查返回值 | 返回true，故障激活 |
| DIAG-022 | APP_DIAG_CHG_MCUTempFaultProcess | MCU过温恢复 | 1. 先触发过温故障 2. 降低currentTemp到75（低于恢复阈值） 3. 持续调用超过恢复延时 | 故障恢复，返回false |
| DIAG-023 | APP_DIAG_CHG_isMCUTempFaultActive | 获取MCU过温状态 | 1. 触发MCU过温 2. 调用isMCUTempFaultActive | 返回true |
| DIAG-024 | APP_DIAG_CHG_HandleOverMCUTempProtection | MCU过温故障处理 | 1. 输入boardFault=true 2. 调用HandleOverMCUTempProtection | 正确上报充电模块过温故障事件 |
| DIAG-025 | APP_DIAG_CHG_RelayFault_Init | 继电器粘连初始化 | 1. 传入faultDelayMs=200, recoverDelayMs=1000 2. 调用初始化 | 延时参数正确设置 |
| DIAG-026 | APP_DIAG_CHG_RelayFaultProcess | 继电器正常-断开 | 1. 输入currentRelayStatus=false（继电器断开） 2. 调用FaultProcess | 无故障触发 |
| DIAG-027 | APP_DIAG_CHG_RelayFaultProcess | 继电器粘连检测 | 1. 输入currentRelayStatus=true（继电器异常闭合） 2. 持续调用超过延时 | 继电器粘连故障标志置位 |
| DIAG-028 | APP_DIAG_CHG_isRelayFaultActive | 获取继电器故障状态-无 | 1. 无继电器故障 2. 调用isRelayFaultActive | 返回false |
| DIAG-029 | APP_DIAG_CHG_isRelayFaultActive | 获取继电器故障状态-有 | 1. 触发继电器粘连故障 2. 调用isRelayFaultActive | 返回true |
| DIAG-030 | APP_DIAG_CHG_HandleRelayProtection | 继电器故障处理 | 1. 输入relayFault=true 2. 调用HandleRelayProtection | 正确上报继电器故障事件 |
| DIAG-031 | APP_DIAG_CHG_CurProtector_Init2 | 过流检测初始化 | 1. 调用CurProtector_Init2 2. 检查过流检测参数 | 参数清零，阈值初始化 |
| DIAG-032 | APP_DIAG_CHG_CurProcess | 电流正常-不过流 | 1. 输入measuredCurrent=5A（低于阈值） 2. 调用CurProcess | 无过流故障 |
| DIAG-033 | APP_DIAG_CHG_CurProcess | 过流触发 | 1. 输入measuredCurrent=15A（超过阈值） 2. 持续调用超过延时 | 过流故障标志置位 |
| DIAG-034 | APP_DIAG_CHG_CurIsFaultActive | 获取过流状态-无 | 1. 电流正常 2. 调用CurIsFaultActive | 返回false |
| DIAG-035 | APP_DIAG_CHG_CurIsFaultActive | 获取过流状态-有 | 1. 触发过流 2. 调用CurIsFaultActive | 返回true |
| DIAG-036 | APP_DIAG_CHG_CurIsChargingLocked | 过流锁定状态 | 1. 过流触发后达到锁定条件 2. 调用CurIsChargingLocked | 返回true（已锁定） |
| DIAG-037 | APP_DIAG_CHG_CurReset | 过流状态机重置 | 1. 触发过流故障 2. 调用CurReset 3. 检查状态 | 过流状态机复位到初始状态 |
| DIAG-038 | APP_DIAG_CHG_CurHandleOverCurrentProtection | 过流故障处理 | 1. 输入currentFault=true, currentLocked=false 2. 调用HandleOverCurrentProtection | 正确上报过流告警事件 |
| DIAG-039 | APP_DIAG_CHG_checkVoltageStatus | 电压正常 | 1. 输入voltage=220000（正常值） 2. 调用checkVoltageStatus | 返回VOLTAGE_NORMAL |
| DIAG-040 | APP_DIAG_CHG_checkVoltageStatus | 过压 | 1. 输入voltage=280000（超过过压阈值275000） 2. 调用 | 返回VOLTAGE_OVER |
| DIAG-041 | APP_DIAG_CHG_checkVoltageStatus | 欠压 | 1. 输入voltage=70000（低于欠压阈值80000） 2. 调用 | 返回VOLTAGE_UNDER |
| DIAG-042 | APP_DIAG_CHG_VoltageProtectionHandler | 过压持续触发 | 1. 输入过压值 2. 持续调用超过DIAGNOSTIC_CHG_ABNORMAL_TIME_THRESHOLD(200ms) | 过压故障标志置位 |
| DIAG-043 | APP_DIAG_CHG_VoltageProtectionHandler | 欠压持续触发 | 1. 输入欠压值 2. 持续调用超过200ms | 欠压故障标志置位 |
| DIAG-044 | APP_DIAG_CHG_VoltageProtectionGetAlarmStatus | 获取电压报警-无 | 1. 电压正常 2. 调用GetAlarmStatus | 返回false |
| DIAG-045 | APP_DIAG_CHG_VoltageProtectionGetAlarmStatus | 获取电压报警-有 | 1. 触发过压 2. 调用GetAlarmStatus | 返回true |
| DIAG-046 | APP_DIAG_CHG_HandleVoltageProtection | 过欠压故障处理 | 1. 输入voltageFault=true 2. 调用HandleVoltageProtection | 正确上报过欠压故障事件 |
| DIAG-047 | APP_DIAG_CHG_GroundFaultHandler | 接地正常 | 1. 输入currentGroundStatus=0（正常） 2. 调用GroundFaultHandler | 无故障触发 |
| DIAG-048 | APP_DIAG_CHG_GroundFaultHandler | 接地故障持续 | 1. 输入currentGroundStatus=1（异常） 2. 持续调用超过200ms | 接地故障锁定 |
| DIAG-049 | APP_DIAG_CHG_GroundFaultIsLocked | 接地故障锁定判断-未锁定 | 1. 接地正常 2. 调用GroundFaultIsLocked | 返回false |
| DIAG-050 | APP_DIAG_CHG_GroundFaultIsLocked | 接地故障锁定判断-已锁定 | 1. 触发接地故障超过阈值 2. 调用GroundFaultIsLocked | 返回true |
| DIAG-051 | APP_DIAG_CHG_HandleGroundProtection | 接地故障处理 | 1. 输入groundFault=true 2. 调用HandleGroundProtection | 正确上报接地故障事件 |

### 3.3 内部函数 — 放电诊断

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| DIAG-052 | APP_DIAG_DISCHG_MCUTempFaultProcess | 放电MCU过温 | 1. 输入currentTemp=105（超过阈值） 2. 持续调用超过延时 | 故障激活，返回true |
| DIAG-053 | APP_DIAG_DISCHG_isMCUTempFaultActive | 获取放电MCU过温状态 | 1. 触发过温 2. 调用isMCUTempFaultActive | 返回true |
| DIAG-054 | APP_DIAG_DISCHG_HandleOverMCUTempProtection | 放电MCU过温处理 | 1. 输入boardFault=true 2. 调用 | 上报放电过温故障 |
| DIAG-055 | APP_DIAG_DISCHG_NTC1TempFaultProcess | NTC1过温触发 | 1. 输入currentTemp超过NTC1过温阈值 2. 持续调用 | NTC1过温故障激活 |
| DIAG-056 | APP_DIAG_DISCHG_isNTC1TempFaultActive | 获取NTC1过温状态 | 1. 触发NTC1过温 2. 调用 | 返回true |
| DIAG-057 | APP_DIAG_DISCHG_HandleOverNTC1TempProtection | NTC1过温处理 | 1. 输入boardFault=true 2. 调用 | 上报NTC1过温故障 |
| DIAG-058 | APP_DIAG_DISCHG_CurProtector_Init2 | 放电过流初始化 | 1. 调用Init2 2. 检查参数 | 参数清零 |
| DIAG-059 | APP_DIAG_DISCHG_CurProcess | 放电正常电流 | 1. 输入正常电流值 2. 调用CurProcess | 无过流故障 |
| DIAG-060 | APP_DIAG_DISCHG_CurProcess | 放电过流触发 | 1. 输入超过阈值电流 2. 持续调用 | 过流故障标志置位 |
| DIAG-061 | APP_DIAG_DISCHG_CurIsFaultActive | 获取放电过流状态 | 1. 触发放电过流 2. 调用CurIsFaultActive | 返回true |
| DIAG-062 | APP_DIAG_DISCHG_CurIsChargingLocked | 放电过流锁定 | 1. 过流达到锁定条件 2. 调用CurIsChargingLocked | 返回true |
| DIAG-063 | APP_DIAG_DISCHG_CurReset | 放电过流重置 | 1. 触发过流 2. 调用CurReset 3. 检查状态 | 状态机复位 |
| DIAG-064 | APP_DIAG_DISCHG_CurHandleOverCurrentProtection | 放电过流处理 | 1. 输入currentFault=true 2. 调用 | 上报放电过流故障 |
| DIAG-065 | APP_DIAG_DISCHG_checkVoltageStatus | 放电电压正常 | 1. 输入voltage=220000 2. 调用 | 返回VOLTAGE_NORMAL |
| DIAG-066 | APP_DIAG_DISCHG_checkVoltageStatus | 放电过压 | 1. 输入voltage=270000（超过264000阈值） 2. 调用 | 返回VOLTAGE_OVER |
| DIAG-067 | APP_DIAG_DISCHG_checkVoltageStatus | 放电欠压 | 1. 输入voltage=170000（低于176000阈值） 2. 调用 | 返回VOLTAGE_UNDER |
| DIAG-068 | APP_DIAG_DISCHG_VoltageProtectionHandler | 放电过压持续 | 1. 输入过压值 2. 持续调用超过200ms | 过压故障置位 |
| DIAG-069 | APP_DIAG_DISCHG_VoltageProtectionGetAlarmStatus | 获取放电电压报警 | 1. 触发放电过压 2. 调用 | 返回true |
| DIAG-070 | APP_DIAG_DISCHG_HandleVoltageProtection | 放电过欠压处理 | 1. 输入voltageFault=true 2. 调用 | 上报放电过欠压故障 |

---

## 4 ASW层 — Discharging模块

**核心职责**：通过状态机执行放电输入和输出命令，为放电部分软件应用功能核心。

### 4.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| DCHG-001 | APP_DISCHG_EventPubTopic | 发布放电插枪事件 | 1. 调用APP_DISCHG_EventPubTopic(EVENT_PLUGIN) | 事件成功入队 |
| DCHG-002 | APP_DISCHG_EventPubTopic | 发布放电拔枪事件 | 1. 调用APP_DISCHG_EventPubTopic(EVENT_UNPLUG) | 事件成功入队 |
| DCHG-003 | APP_DISCHG_EventPub | 发布放电告警带故障码 | 1. 调用APP_DISCHG_EventPub(EVENT_ALARM, 0x02) | 事件及故障码成功入队 |
| DCHG-004 | APP_DISCHG_EventPub | 发布放电保护暂停事件 | 1. 调用APP_DISCHG_EventPub(EVENT_PROTECT_PAUSE, code) | 事件及数据成功入队 |
| DCHG-005 | APP_DISCHG_StateMachineDispatch | 空闲状态下调度 | 1. 初始化放电状态机为IDLE 2. 调用Dispatch | 返回OK，状态不变 |
| DCHG-006 | APP_DISCHG_StateMachineDispatch | 有事件时调度 | 1. 发布放电开始事件 2. 调用Dispatch | 状态机正确响应并转换 |
| DCHG-007 | APP_DISCHG_StateMachineInit | 初始化验证 | 1. 调用APP_DISCHG_StateMachineInit 2. 检查状态 | 上一次状态和当前状态均初始化为IDLE |
| DCHG-008 | APP_DISCHG_GetChargeStatus | 初始状态获取 | 1. 初始化放电模块 2. 调用GetChargeStatus | 返回IDLE或默认初始状态 |
| DCHG-009 | APP_DISCHG_GetChargeStatus | 放电中状态获取 | 1. 触发放电流程 2. 调用GetChargeStatus | 返回DISCHARGING状态 |
| DCHG-010 | APP_DISCHG_GetStatusLED | 放电空闲LED | 1. 放电状态为IDLE 2. 调用GetStatusLED | 返回对应空闲LED状态值 |
| DCHG-011 | APP_DISCHG_GetStatusLED | 放电中LED | 1. 放电状态为DISCHARGING 2. 调用GetStatusLED | 返回对应放电LED状态值 |
| DCHG-012 | APP_DISCHG_GetFault | 初始无故障 | 1. 初始化放电模块 2. 调用GetFault | 返回0 |
| DCHG-013 | APP_DISCHG_GetFault | 有放电故障码 | 1. 触发放电过流故障 2. 调用GetFault | 返回对应故障码 |
| DCHG-014 | APP_DISCHG_IsFaultPresent | 特定放电故障存在 | 1. 触发放电过流 2. 调用IsFaultPresent(DISCHG_OVER_CURRENT) | 返回true |
| DCHG-015 | APP_DISCHG_IsFaultPresent | 特定放电故障不存在 | 1. 触发放电过流 2. 调用IsFaultPresent(DISCHG_OVER_VOLTAGE) | 返回false |
| DCHG-016 | APP_DISCHG_IsAnyFaultPresent | 无放电故障 | 1. 初始化后 2. 调用IsAnyFaultPresent | 返回false |
| DCHG-017 | APP_DISCHG_IsAnyFaultPresent | 有放电故障 | 1. 触发任意放电故障 2. 调用IsAnyFaultPresent | 返回true |

---

## 5 ASW层 — LED模块

**核心职责**：直观向用户传递充电设备当前的工作状态。

### 5.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| LED-001 | APP_Led_Init | LED初始化 | 1. 调用APP_Led_Init 2. 检查LED各通道初始状态 | 所有LED通道初始化为默认关闭状态 |
| LED-002 | APP_Led_Init | 重复初始化 | 1. 调用APP_Led_Init两次 2. 检查状态 | 第二次初始化正确重置 |
| LED-003 | Led_MainFunction | 主任务正常执行 | 1. 初始化LED 2. 调用Led_MainFunction 3. 检查LED处理结果 | LED处理函数被正确调用 |
| LED-004 | Led_MainFunction | 连续调用稳定性 | 1. 连续调用Led_MainFunction 100次（模拟1秒） | 每次均正常执行 |

### 5.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| LED-005 | Led_Process | 充电模式处理 | 1. 设置充电模式 2. 调用Led_Process 3. 检查LED输出 | LED显示对应充电状态灯语 |
| LED-006 | Led_Process | 放电模式处理 | 1. 设置放电模式 2. 调用Led_Process 3. 检查LED输出 | LED显示对应放电状态灯语 |
| LED-007 | Led_auto | LED自动执行-绿灯 | 1. 设置Led_info为绿色LED 2. 调用Led_auto 3. 检查GPIO输出 | 绿色LED GPIO正确输出 |
| LED-008 | Led_auto | LED自动执行-红灯 | 1. 设置Led_info为红色LED 2. 调用Led_auto | 红色LED GPIO正确输出 |
| LED-009 | Led_auto | LED自动执行-蓝灯 | 1. 设置Led_info为蓝色LED 2. 调用Led_auto | 蓝色LED GPIO正确输出 |
| LED-010 | Led_ShowLoop | 自检灯语循环 | 1. 调用Led_ShowLoop 2. 验证各颜色LED依次点亮 | RGB三色灯依次循环点亮 |
| LED-011 | LED_CHGMODE_Showfault | 充电过流故障灯语 | 1. 输入fault_type为过流故障 2. 调用Showfault | LED显示过流故障对应灯语（红灯闪烁） |
| LED-012 | LED_CHGMODE_Showfault | 充电过温故障灯语 | 1. 输入fault_type为过温故障 2. 调用Showfault | LED显示过温故障灯语 |
| LED-013 | LED_DISCHGMODE_Showfault | 放电故障灯语 | 1. 输入fault_type为放电故障 2. 调用Showfault | LED显示放电故障对应灯语 |
| LED-014 | Led_Set_Mode | 设置常亮模式 | 1. 调用Led_Set_Mode(led, LED_MODE_ON) 2. 检查LED状态 | LED常亮 |
| LED-015 | Led_Set_Mode | 设置闪烁模式 | 1. 调用Led_Set_Mode(led, LED_MODE_BLINK) 2. 多次调用检查 | LED按频率闪烁 |
| LED-016 | Led_Set_Mode | 设置关闭模式 | 1. 调用Led_Set_Mode(led, LED_MODE_OFF) 2. 检查LED状态 | LED关闭 |

---

## 6 ASW层 — Factory模块

**核心职责**：满足出厂前测试等各项需求。

### 6.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| FAC-001 | fatcory | 进入工厂模式命令 | 1. 构造argv={"factory", "entry"} 2. 调用fatcory(2, argv) | 进入工厂模式，模式状态置位 |
| FAC-002 | fatcory | 退出工厂模式命令 | 1. 构造argv={"factory", "exit"} 2. 调用fatcory(2, argv) | 退出工厂模式，模式状态清除 |
| FAC-003 | fatcory | 无效命令 | 1. 构造argv={"factory", "invalid"} 2. 调用fatcory(2, argv) | 不改变工厂模式状态 |
| FAC-004 | fatcory | 参数不足 | 1. 构造argv={"factory"} 2. 调用fatcory(1, argv) | 不崩溃，安全返回 |

### 6.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| FAC-005 | factory_entry_mode | 进入工厂模式 | 1. 调用factory_entry_mode 2. 检查工厂模式状态 | 工厂模式激活，相关IO设置正确 |
| FAC-006 | factory_exit_mode | 退出工厂模式 | 1. 先进入工厂模式 2. 调用factory_exit_mode 3. 检查状态 | 工厂模式退出，状态清除 |
| FAC-007 | get_factory_mode_status | 工厂模式状态查询-未进入 | 1. 初始状态 2. 调用get_factory_mode_status | 返回false |
| FAC-008 | get_factory_mode_status | 工厂模式状态查询-已进入 | 1. 进入工厂模式 2. 调用get_factory_mode_status | 返回true |

---

## 7 ASW层 — Log模块

**核心职责**：监控产品各模块工作状态信息。

### 7.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| LOG-001 | Log_Printf_Main | Log主函数正常执行 | 1. 初始化Log模块 2. 调用Log_Printf_Main 3. 检查串口输出 | 正确输出各模块状态信息 |
| LOG-002 | Log_Printf_Main | 连续调用稳定性 | 1. 连续调用Log_Printf_Main 100次 | 每次均正常执行，无内存泄漏 |
| LOG-003 | Log_Printf_Main | 无数据时执行 | 1. 无待输出日志 2. 调用Log_Printf_Main | 安全返回，不崩溃 |

---

## 8 ASW层 — Calibration模块

**核心职责**：通过注入方式对电压电流进行校准，校准参数存储在flash内。

### 8.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CAL-001 | Calibration_Update | 进入校准模式 | 1. 设置state=CLAIBRATION_ENTRY 2. 调用Calibration_Update | 继电器断开，短路检测IO断开，状态转为CALIBRATION_WAIT |
| CAL-002 | Calibration_Update | 校准等待阶段 | 1. 设置state=CALIBRATION_WAIT 2. 调用Calibration_Update 3. 等待200ms | 状态转为CALIBRATION_INTO，timer=0 |
| CAL-003 | Calibration_Update | 校准进行中 | 1. 设置state=CALIBRATION_INTO 2. 调用Calibration_Update | timer清零，校准参数可正常设置 |
| CAL-004 | Calibration_Update | 退出校准模式 | 1. 设置state=CALIBRATION_EXIT 2. 调用Calibration_Update | 触发NVIC_SystemReset系统复位 |
| CAL-005 | inject | 注入测试命令 | 1. 构造argv={"inject", "param"} 2. 调用inject | 正确解析并执行注入命令 |
| CAL-006 | inject | 注入参数不足 | 1. 构造argv={"inject"} 2. 调用inject(1, argv) | 安全返回，不崩溃 |
| CAL-007 | calibration | 电压校准命令 | 1. 构造argv={"calibration", "vol", "k", "b"} 2. 调用calibration | 电压校准参数k、b设置成功 |
| CAL-008 | calibration | 电流校准命令 | 1. 构造argv={"calibration", "cur", "k", "b"} 2. 调用calibration | 电流校准参数k、b设置成功 |
| CAL-009 | calibration | 无效校准类型 | 1. 构造argv={"calibration", "invalid"} 2. 调用calibration | 不修改任何校准参数 |
| CAL-010 | restart | 复位命令 | 1. 构造argv={"restart"} 2. 调用restart | 触发NVIC软件复位 |
| CAL-011 | relay | 继电器闭合命令 | 1. 构造argv={"relay", "on"} 2. 调用relay | 继电器闭合 |
| CAL-012 | relay | 继电器断开命令 | 1. 构造argv={"relay", "off"} 2. 调用relay | 继电器断开 |

### 8.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CAL-013 | calibration_vol_set | 设置电压校准参数 | 1. 调用calibration_vol_set(cal, k=1000, b=50) 2. 检查k、b值 | 电压k=1000, b=50设置成功 |
| CAL-014 | calibration_vol_set | 边界值-k最大 | 1. 调用calibration_vol_set(cal, k=0xFFFFFFFF, b=0) | 不溢出，k值正确设置 |
| CAL-015 | calibration_vol_clear | 清除电压校准参数 | 1. 先设置参数 2. 调用calibration_vol_clear 3. 检查k、b值 | k和b清零或恢复默认 |
| CAL-016 | calibration_cur_set | 设置电流校准参数 | 1. 调用calibration_cur_set(cal, k=2000, b=100) 2. 检查k、b值 | 电流k=2000, b=100设置成功 |
| CAL-017 | calibration_cur_clear | 清除电流校准参数 | 1. 先设置参数 2. 调用calibration_cur_clear 3. 检查k、b值 | k和b清零或恢复默认 |
| CAL-018 | get_calibration_mode_status | 非校准模式 | 1. 初始状态 2. 调用get_calibration_mode_status | 返回0（非校准模式） |
| CAL-019 | get_calibration_mode_status | 校准模式 | 1. 进入校准模式 2. 调用get_calibration_mode_status | 返回1或2（校准模式中） |

---

## 9 ASW层 — 电压测量模块

**核心职责**：通过RMS计算电压值作为过欠压判断条件。

### 9.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| VOL-001 | AdcM_GetValidValue | 获取电压通道值 | 1. 设置ADCM_Voltage_Val=220000 2. 传入Channel=ADCMCFG_Voltage_CH 3. 调用GetValidValue | 返回220000 |
| VOL-002 | AdcM_GetValidValue | 无效通道 | 1. 传入无效Channel值 2. 调用GetValidValue | 返回0或安全默认值，不崩溃 |
| VOL-003 | ADCM_CallbackSampleValue | 电压数据计算处理 | 1. 模拟ADC采样缓冲区有数据 2. 调用ADCM_CallbackSampleValue 3. 检查计算结果 | RMS电压计算值正确 |

### 9.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| VOL-004 | AdcM_get_V_LINE_MCU_update | 正常电压采样值更新 | 1. 输入Samp_Val=2048（中点值） 2. 调用update 3. 检查缓冲区 | 采样值正确写入缓冲区 |
| VOL-005 | AdcM_get_V_LINE_MCU_update | 最大采样值 | 1. 输入Samp_Val=65535 2. 调用update | 采样值正确写入 |
| VOL-006 | AdcM_get_V_LINE_MCU_update | 最小采样值 | 1. 输入Samp_Val=0 2. 调用update | 采样值正确写入 |
| VOL-007 | AdcM_cal_V_LINE_MCU_update | 正弦波RMS计算 | 1. 填充完整正弦波采样数据 2. 调用cal_update 3. 检查RMS值 | RMS值接近正弦波有效值 |
| VOL-008 | AdcM_cal_V_LINE_MCU_update | 采样点不足时不计算 | 1. 缓冲区数据不足ADC_LIST_VOL_LOWER_LEN(5) 2. 调用cal_update | 不执行RMS计算，结果不变 |

---

## 10 ASW层 — 电流测量模块

**核心职责**：通过RMS计算电流值作为过流判断条件。

### 10.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CUR-001 | AdcM_GetValidValue | 获取电流通道值 | 1. 设置ADCM_Current_Val=5000 2. 传入Channel=ADCMCFG_Current_CH 3. 调用 | 返回5000 |
| CUR-002 | AdcM_GetValidValue | 无效通道 | 1. 传入无效Channel 2. 调用 | 返回0或安全默认值 |
| CUR-003 | ADCM_CallbackSampleValue | 电流数据计算处理 | 1. 模拟ADC电流采样数据 2. 调用CallbackSampleValue 3. 检查电流计算结果 | RMS电流计算值正确 |

### 10.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| CUR-004 | AdcM_get_I_CS_update | 正常电流采样更新 | 1. 输入Samp_Val=2048 2. 调用update | 采样值正确写入缓冲区 |
| CUR-005 | AdcM_get_I_CS_update | 边界采样值 | 1. 输入Samp_Val=0和65535 2. 分别调用update | 均正确写入 |
| CUR-006 | AdcM_cal_I_CS_update | 正弦波RMS电流计算 | 1. 填充完整正弦波电流数据 2. 调用cal_update | RMS电流值接近有效值 |
| CUR-007 | AdcM_cal_I_CS_update | 采样点不足时不计算 | 1. 数据不足ADC_LIST_CUR_LOWER_ZEROLINE(14) 2. 调用 | 不执行计算 |

---

## 11 BSW层 — AdcF模块

**核心职责**：实现ADC各通道采样值的滤波处理。

### 11.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| ADCF-001 | sortDesc | 正常数组降序排序 | 1. 输入arr={3,1,4,1,5,9,2,6} 2. 调用sortDesc(arr, 8) 3. 验证结果 | arr={9,6,5,4,3,2,1,1} |
| ADCF-002 | sortDesc | 空数组 | 1. 调用sortDesc(arr, 0) | 不崩溃 |
| ADCF-003 | sortDesc | 单元素数组 | 1. 输入arr={5} 2. 调用sortDesc(arr, 1) | arr={5}不变 |
| ADCF-004 | sortDesc | 已排序数组 | 1. 输入arr={5,4,3,2,1} 2. 调用sortDesc(arr, 5) | arr={5,4,3,2,1}不变 |
| ADCF-005 | sortDesc | 全相同元素 | 1. 输入arr={3,3,3,3} 2. 调用sortDesc(arr, 4) | arr={3,3,3,3}不变 |
| ADCF-006 | AdcF_CulateIntAverage_Conversion | 正常平均值计算 | 1. 输入data={100,200,300}，length=3 2. 调用计算函数 | 返回200（截断平均值） |
| ADCF-007 | AdcF_CulateIntAverage_Conversion | 去极值后平均值 | 1. 输入含极值的数据 2. 先排序去极值再调用 3. 检查结果 | 去极值后平均值正确 |
| ADCF-008 | AdcF_CulateIntAverage_Conversion | 长度为0 | 1. 输入length=0 2. 调用 | 返回0或安全值，不除零 |
| ADCF-009 | AdcF_CulateIntAverage_Conversion | 长度为1 | 1. 输入data={42}，length=1 2. 调用 | 返回42 |
| ADCF-010 | FastSqrt | 完全平方数 | 1. 输入input=144 2. 调用FastSqrt | 返回12 |
| ADCF-011 | FastSqrt | 非完全平方数 | 1. 输入input=200 2. 调用FastSqrt | 返回14（√200≈14.14，近似值） |
| ADCF-012 | FastSqrt | 输入为0 | 1. 输入input=0 2. 调用FastSqrt | 返回0 |
| ADCF-013 | FastSqrt | 输入为1 | 1. 输入input=1 2. 调用FastSqrt | 返回1 |
| ADCF-014 | FastSqrt | 大数输入 | 1. 输入input=4294967295（uint32最大值） 2. 调用 | 返回65535（√max≈65535.99） |
| ADCF-015 | hard_root | 正常开方 | 1. 输入ori=14400 2. 调用hard_root | 返回120 |
| ADCF-016 | hard_root | 输入为0 | 1. 输入ori=0 2. 调用hard_root | 返回0 |
| ADCF-017 | hard_root | 大数开方 | 1. 输入大数 2. 调用hard_root 3. 与sqrt库函数比较 | 误差在可接受范围内 |
| ADCF-018 | threadHold_check_u32 | q1-q2在阈值内 | 1. 输入q1=100, q2=105, threadhold=10 2. 调用 | 返回true（差值5<10） |
| ADCF-019 | threadHold_check_u32 | q1-q2超过阈值 | 1. 输入q1=100, q2=120, threadhold=10 2. 调用 | 返回false（差值20>10） |
| ADCF-020 | threadHold_check_u32 | 恰好等于阈值 | 1. 输入q1=100, q2=110, threadhold=10 2. 调用 | 按设计返回（边界判定） |
| ADCF-021 | threadHold_check_u32 | q2>q1的情况 | 1. 输入q1=105, q2=100, threadhold=10 2. 调用 | 正确处理绝对值差 |
| ADCF-022 | threadHold_check_u32 | 阈值为0 | 1. 输入q1=100, q2=100, threadhold=0 2. 调用 | 差值为0时返回true |

---

## 12 BSW层 — AdcM模块

**核心职责**：实现ADC各通道采样值的滤波处理（含电压、电流、CP反馈、NTC1温度、接地、二极管、继电器、MCU温度、短路等通道）。

### 12.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| ADCM-001 | AdcM_GetValidValue | 获取电压通道 | 1. 传入Channel=ADCMCFG_Voltage_CH 2. 调用 | 返回电压计算值 |
| ADCM-002 | AdcM_GetValidValue | 获取电流通道 | 1. 传入Channel=ADCMCFG_Current_CH 2. 调用 | 返回电流计算值 |
| ADCM-003 | AdcM_GetValidValue | 获取CP反馈通道 | 1. 传入Channel=ADCMCFG_CP_FB_AMP_CH 2. 调用 | 返回CP反馈值 |
| ADCM-004 | AdcM_GetValidValue | 获取温度通道 | 1. 传入Channel=ADCMCFG_NTC1_CH 2. 调用 | 返回NTC1温度值 |
| ADCM-005 | AdcM_GetValidValue | 无效通道 | 1. 传入无效Channel值 2. 调用 | 安全返回，不崩溃 |
| ADCM-006 | ADCM_CallbackSampleValue | 全通道数据计算 | 1. 模拟所有ADC通道有数据 2. 调用CallbackSampleValue 3. 检查各通道值 | 各通道计算值正确更新 |

### 12.2 内部函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| ADCM-007 | AdcM_get_V_LINE_MCU_update | 电压采样写入 | 1. 输入Samp_Val=2048 2. 调用 | 采样值正确写入电压缓冲区 |
| ADCM-008 | AdcM_cal_V_LINE_MCU_update | 电压RMS计算 | 1. 填充正弦波数据 2. 调用cal_update | 电压RMS值正确 |
| ADCM-009 | AdcM_get_I_CS_update | 电流采样写入 | 1. 输入Samp_Val=2048 2. 调用 | 采样值正确写入电流缓冲区 |
| ADCM-010 | AdcM_cal_I_CS_update | 电流RMS计算 | 1. 填充正弦波数据 2. 调用cal_update | 电流RMS值正确 |
| ADCM-011 | AdcM_get_CP_FB_AMP_update | CP反馈采样 | 1. 输入Samp_Val=1000 2. 调用 | 采样值正确写入CP缓冲区 |
| ADCM-012 | AdcM_cal_CP_FB_AMP_update | CP反馈计算 | 1. 填充CP采样数据 2. 调用cal_update | CP反馈计算值正确 |
| ADCM-013 | AdcM_get_TEMP_NTC1_update | NTC1温度采样 | 1. 输入Samp_Val=1500 2. 调用 | 采样值正确写入NTC1缓冲区 |
| ADCM-014 | getTempInterp | 查表法正常温度 | 1. 输入targetVol对应25°C 2. 调用getTempInterp | 返回25°C附近温度值 |
| ADCM-015 | getTempInterp | 查表法极端低温 | 1. 输入targetVol对应-40°C 2. 调用 | 返回-40°C附近值 |
| ADCM-016 | getTempInterp | 查表法极端高温 | 1. 输入targetVol对应125°C 2. 调用 | 返回125°C附近值 |
| ADCM-017 | getTempInterp | 超出查表范围 | 1. 输入targetVol超出表范围 2. 调用 | 返回边界值，不崩溃 |
| ADCM-018 | AdcM_calTEMP_NTC1_update | NTC1温度计算 | 1. 填充NTC1采样数据 2. 调用cal_update | 温度计算值合理 |
| ADCM-019 | AdcM_get_PGD_update | 接地采样写入 | 1. 输入Samp_Val=100 2. 调用 | 采样值正确写入接地缓冲区 |
| ADCM-020 | AdcM_cal_PGD_update | 接地正常计算 | 1. 模拟正常接地采样 2. 调用cal_update | 接地状态为OK(0) |
| ADCM-021 | AdcM_cal_PGD_update | 接地故障计算 | 1. 模拟异常接地采样超过阈值 2. 调用cal_update | 接地状态为ERR(1) |
| ADCM-022 | AdcM_get_EVDIODE_CHK_update | 二极管采样写入 | 1. 输入Samp_Val=2000 2. 调用 | 采样值正确写入二极管缓冲区 |
| ADCM-023 | AdcM_cal_EVDIODE_CHK_update | 二极管计算 | 1. 填充二极管采样数据 2. 调用cal_update | 二极管计算值正确 |
| ADCM-024 | AdcM_get_RLY_CHK_MCU_update | 继电器采样写入 | 1. 输入Samp_Val=3000 2. 调用 | 采样值正确写入继电器缓冲区 |
| ADCM-025 | AdcM_cal_RLY_CHK_MCU_update | 继电器正常-断开 | 1. 模拟继电器断开采样 2. 调用cal_update | 状态为BREAK(1) |
| ADCM-026 | AdcM_cal_RLY_CHK_MCU_update | 继电器闭合 | 1. 模拟继电器闭合采样 2. 调用cal_update | 状态为CONNECT(0) |
| ADCM-027 | AdcM_get_MCU_Temp_update | MCU温度采样 | 1. 输入Samp_Val=1800 2. 调用 | 采样值正确写入MCU温度缓冲区 |
| ADCM-028 | AdcM_cal_MCU_Temp_update | MCU温度计算 | 1. 填充MCU温度采样数据 2. 调用cal_update | MCU温度值合理（25~50°C范围） |
| ADCM-029 | AdcM_get_ShortCircuit_update | 短路采样写入 | 1. 输入Samp_Val=500 2. 调用 | 采样值正确写入短路缓冲区 |
| ADCM-030 | AdcM_cal_ShortCircuit_update | 短路正常 | 1. 模拟正常短路采样 2. 调用cal_update | 状态为OK(0) |
| ADCM-031 | AdcM_cal_ShortCircuit_update | 短路故障 | 1. 模拟异常短路采样超过阈值 2. 调用cal_update | 状态为ERR(1) |

---

## 13 BSW层 — EcuM模块

**核心职责**：实现软件各模块的初始化功能，用于软件的启动运行。

### 13.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| ECUM-001 | EcuM_InitOne | 驱动层初始化 | 1. 调用EcuM_InitOne 2. 检查各驱动初始化函数是否被调用 | 所有驱动模块初始化函数均被调用 |
| ECUM-002 | EcuM_InitOne | 重复调用 | 1. 调用EcuM_InitOne两次 | 第二次初始化不崩溃 |
| ECUM-003 | EcuM_InitTwo | 应用层初始化 | 1. 调用EcuM_InitTwo 2. 检查各应用层模块初始化 | 充电、放电、CP、Diag、LED等模块初始化被调用 |
| ECUM-004 | EcuM_InitTwo | 重复调用 | 1. 调用EcuM_InitTwo两次 | 第二次初始化不崩溃 |
| ECUM-005 | EcuM_InitThree | 其他模块初始化 | 1. 调用EcuM_InitThree 2. 检查其他模块初始化 | OS启动、看门狗等初始化被调用 |
| ECUM-006 | EcuM_InitThree | 重复调用 | 1. 调用EcuM_InitThree两次 | 第二次初始化不崩溃 |
| ECUM-007 | ECUM_StartOS | 启动OS | 1. 调用ECUM_StartOS 2. 检查OS是否启动 | OS正确启动 |

---

## 14 BSW层 — WdgM模块

**核心职责**：实现看门狗的喂狗功能，保证程序正常运行，异常时对MCU进行复位。

### 14.1 外部接口函数

| 用例编号 | 函数名 | 测试用例名称 | 测试流程概述 | 预期结果 |
|---------|--------|-------------|-------------|---------|
| WDGM-001 | WdgM_Init | 看门狗初始化 | 1. 调用WdgM_Init 2. 检查看门狗外设配置 | 看门狗外设正确配置，超时时间设置正确 |
| WDG-002 | WdgM_Init | 重复初始化 | 1. 调用WdgM_Init两次 | 第二次初始化不崩溃 |
| WDGM-003 | WdgM_Trigger | 正常喂狗 | 1. 初始化看门狗 2. 调用WdgM_Trigger 3. 检查看门狗计数器 | 看门狗计数器被重置 |
| WDGM-004 | WdgM_Trigger | 周期喂狗 | 1. 初始化看门狗 2. 在超时时间内周期调用WdgM_Trigger 3. 检查系统不复位 | 系统正常运行，不复位 |
| WDGM-005 | WdgM_Trigger | 未及时喂狗 | 1. 初始化看门狗 2. 不调用WdgM_Trigger 3. 等待超时 | 系统触发复位（需在集成测试中验证） |

---

## 附录A：测试用例统计

| 模块分类 | 模块名称 | 外部接口函数数 | 内部函数数 | 用例总数 |
|---------|---------|-------------|-----------|---------|
| ASW | CP模块 | 4 | 5 | 31 |
| ASW | Charging模块 | 10 | 0 | 26 |
| ASW | Diag模块 | 7 | 53 | 70 |
| ASW | Discharging模块 | 9 | 0 | 17 |
| ASW | LED模块 | 2 | 6 | 16 |
| ASW | Factory模块 | 1 | 3 | 8 |
| ASW | Log模块 | 1 | 0 | 3 |
| ASW | Calibration模块 | 5 | 5 | 19 |
| ASW | 电压测量模块 | 2 | 2 | 8 |
| ASW | 电流测量模块 | 2 | 2 | 7 |
| BSW | AdcF模块 | 5 | 0 | 22 |
| BSW | AdcM模块 | 2 | 19 | 31 |
| BSW | EcuM模块 | 4 | 0 | 7 |
| BSW | WdgM模块 | 2 | 0 | 5 |
| **合计** | **14个模块** | **56** | **95** | **270** |

---

## 附录B：测试用例设计原则

1. **正常路径测试**：验证函数在正常输入条件下的正确输出
2. **边界值测试**：验证函数在输入参数边界值时的行为（如0、最大值、阈值±1）
3. **异常路径测试**：验证函数在异常输入下的安全处理（如空指针、无效参数）
4. **状态机测试**：验证状态转换的正确性（所有状态→所有合法事件→预期新状态）
5. **持续条件测试**：验证延时触发类逻辑（持续异常超过阈值才触发）
6. **恢复测试**：验证故障恢复逻辑（故障条件消除后是否正确恢复）
7. **稳定性测试**：验证函数在高频调用下不崩溃、不泄漏
