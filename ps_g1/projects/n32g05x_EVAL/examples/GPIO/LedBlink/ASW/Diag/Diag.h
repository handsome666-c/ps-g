#ifndef _DIAG_H
#define _DIAG_H

#include "n32g05x.h"
#include "RTE_BSW.h"
#include "Charging.h"
#include "Rly.h"

/****************************************充电诊断部分****************************************/
/****************************************漏电****************************************/
// 自检状态定义
typedef enum {
    DIAGNOSTIC_CHG_LeakageCheck_IDLE = 0,      // 空闲状态
    DIAGNOSTIC_CHG_LeakageCheck_START,
} Diagnostic_Charging_LeakageCheckState;

typedef struct {
    Diagnostic_Charging_LeakageCheckState currentState;
    uint32_t count;
    bool Leakage_SCR_Status;
} Diagnostic_Charging_LeakageCheck_Manager_t;

/****************************************短路****************************************/
#define DIAG_CHG_SHORTCIRCUIT_TIMER 8640000 // 5ms*8640000 = 43200000 12小时
typedef enum {
    DIAGNOSTIC_CHG_SHORTCIRCUIT_IDLE = 0,      // 空闲状态
    DIAGNOSTIC_CHG_SHORTCIRCUIT_START,
    DIAGNOSTIC_CHG_SHORTCIRCUIT_CHECKING,
} Diagnostic_Charging_ShortCircuitState;

typedef struct {
    Diagnostic_Charging_ShortCircuitState currentState;
    uint32_t timer;
    bool faultStatus;
} Diagnostic_Charging_ShortCircuit_Manager_t;

/****************************************继电器黏连****************************************/
// 继电器故障状态定义
typedef enum {
    DIAGNOSTIC_CHG_RELAY_STATE_NORMAL = 0,      // 正常状态
    DIAGNOSTIC_CHG_RELAY_STATE_FAULT_DETECTING, // 故障检测中
    DIAGNOSTIC_CHG_RELAY_STATE_FAULT,           // 故障状态（粘连）
    DIAGNOSTIC_CHG_RELAY_STATE_RECOVERING      // 恢复中
} Diagnostic_Charging_RelayFaultState;

typedef struct {
    Diagnostic_Charging_RelayFaultState currentState;
    uint32_t faultDurationMs;           // 故障确认持续时间(ms)
    uint32_t recoverDurationMs;         // 故障恢复持续时间(ms)
    uint32_t faultCounterMs;            // 故障计时器
    uint32_t recoverCounterMs;          // 恢复计时器
    bool isFaultActive;                 // 故障激活标志
    bool lastRelayStatus;               // 上一次继电器状态
} Diagnostic_Charging_Realy_Manager_t;

/****************************************过流****************************************/
// 过流保护状态定义
typedef enum {
    DIAGNOSTIC_CHG_OC_STATE_NORMAL = 0,        // 正常状态
    DIAGNOSTIC_CHG_OC_STATE_LEVEL1_DETECTING,   // 一段过流检测中
    DIAGNOSTIC_CHG_OC_STATE_LEVEL1_FAULT,        // 一段过流故障
    DIAGNOSTIC_CHG_OC_STATE_LEVEL2_FAULT,        // 二段过流故障
    DIAGNOSTIC_CHG_OC_STATE_LEVEL3_FAULT,        // 三段过流故障
    DIAGNOSTIC_CHG_OC_STATE_LOCKED               // 流程锁死状态
} Diagnostic_Charging_OvercurrentState;

// 恢复检测状态定义
typedef enum {
    DIAGNOSTIC_CHG_OC_RECOVERY_IDLE = 0,        // 恢复空闲状态
    DIAGNOSTIC_CHG_OC_RECOVERY_CHECKING,        // 恢复条件检测中
    DIAGNOSTIC_CHG_OC_RECOVERY_CONDITION_MET,   // 恢复条件满足
    DIAGNOSTIC_CHG_OC_RECOVERY_COUNTDOWN        // 恢复倒计时中
} Diagnostic_Charging_OvercurrentRecoveryState;

// 过流保护配置参数
typedef struct {
    uint32_t cpSetCurrent;              // CP设置电流(A)
    uint32_t level1Threshold;           // 一段过流阈值(A)
    uint32_t level2Threshold;           // 二段过流阈值(A)
    uint32_t level3Threshold;           // 三段过流阈值(A)
    uint32_t level1DurationMs;          // 一段过流持续时间(ms)
    uint32_t level2DurationMs;          // 二段过流响应时间(ms)
    uint32_t level3DurationMs;          // 三段过流响应时间(ms)
    uint32_t recoveryDelayMs;          // 恢复延时(ms)
    uint32_t recoveryCheckMs;          // 恢复检测时间(ms)
    uint8_t maxLevel1Faults;            // 最大一段过流次数
} Diagnostic_Charging_OvercurrentConfig;

// 恢复检测参数
typedef struct {
    Diagnostic_Charging_OvercurrentRecoveryState state;   // 恢复状态
    uint32_t checkCounterMs;             // 恢复检测计时器(ms)
    uint32_t countdownCounterMs;        // 恢复倒计时计时器(ms)
    bool conditionMet;                   // 恢复条件是否满足
    float currentThreshold;              // 恢复电流阈值(A)
} Diagnostic_Charging_OvercurrentRecovery;

typedef struct {
    Diagnostic_Charging_OvercurrentState currentState;
    Diagnostic_Charging_OvercurrentConfig config;      // 配置参数
    Diagnostic_Charging_OvercurrentRecovery recovery;    // 恢复检测器
    uint32_t level1CounterMs;           // 一段过流计时器
    uint32_t level2CounterMs;           // 二段过流计时器
    uint32_t level3CounterMs;           // 三段过流计时器
    uint8_t level1FaultCount;           // 一段过流次数计数
    bool isFaultActive;                 // 故障激活标志
    bool isChargingLocked;              // 充电流程锁死标志
    float currentCurrent;               // 当前电流值(A)
} Diagnostic_Charging_Overcurrent_Manager_t;

/****************************************过欠压****************************************/
// 过欠压保护状态定义
typedef enum {
    DIAGNOSTIC_CHG_VOLTAGE_STATE_NORMAL = 0,        // 正常状态
    DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_ALARM,        // 过压报警状态
    DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_ALARM,      // 欠压报警状态
    DIAGNOSTIC_CHG_VOLTAGE_STATE_OVER_RECOVERY,    // 过压恢复中状态
    DIAGNOSTIC_CHG_VOLTAGE_STATE_UNDER_RECOVERY    // 欠压恢复中状态
} Diagnostic_Charging_VoltageState;

// 电压状态检测枚举
typedef enum {
    DIAGNOSTIC_CHG_VOLTAGE_STATUS_NORMAL = 0,
    DIAGNOSTIC_CHG_VOLTAGE_STATUS_OVER,
    DIAGNOSTIC_CHG_VOLTAGE_STATUS_UNDER,
    DIAGNOSTIC_CHG_VOLTAGE_STATUS_IN_RECOVERY_ZONE
} Diagnostic_Charging_VoltageStatus;

// 电压阈值定义
#define DIAGNOSTIC_CHG_OVER_VOLTAGE_THRESHOLD       275000    // 充电过压阈值275V
#define DIAGNOSTIC_CHG_OVER_VOLTAGE_RECOVERY        265000    // 充电过压恢复阈值265V
#define DIAGNOSTIC_CHG_UNDER_VOLTAGE_THRESHOLD      80000     // 充电欠压阈值80V
#define DIAGNOSTIC_CHG_UNDER_VOLTAGE_RECOVERY       90000     // 充电欠压恢复阈值90V

// 时间阈值定义（基于5ms定时器）
#define DIAGNOSTIC_CHG_ABNORMAL_TIME_THRESHOLD      200       // 1s = 200 * 5ms（200次调用）
#define DIAGNOSTIC_CHG_RECOVERY_TIME_THRESHOLD      1000      // 5s = 1000 * 5ms（1000次调用）

typedef struct {
    Diagnostic_Charging_VoltageState currentState;
    uint16_t timerCount;                // 通用计时器（5ms为单位）
    bool alarmStatus;                   // 报警状态
    uint16_t lastVoltage;               // 上次电压值
} Diagnostic_Charging_Voltage_Manager_t;

/****************************************MCU过温参数定义****************************************/
// 过温状态定义
typedef enum {
    DIAGNOSTIC_CHG_MCUTEMP_STATE_NORMAL = 0,       // 温度正常状态
    DIAGNOSTIC_CHG_MCUTEMP_STATE_OVERHEATING,      // 过温检测中状态
    DIAGNOSTIC_CHG_MCUTEMP_STATE_FAULT,            // 故障状态（过温报警）
    DIAGNOSTIC_CHG_MCUTEMP_STATE_COOLING           // 冷却中状态
} Diagnostic_Charging_MCUTempState;

typedef struct {
    Diagnostic_Charging_MCUTempState currentState;
    float overheatThreshold;            // 过温阈值(℃)
    float cooldownThreshold;            // 恢复阈值(℃)
    uint32_t overheatDurationMs;        // 过温持续时间要求(ms)
    uint32_t cooldownDurationMs;       // 冷却持续时间要求(ms)
    uint32_t overheatCounterMs;         // 过温计时器
    uint32_t cooldownCounterMs;        // 冷却计时器
    bool isFaultActive;                 // 故障激活标志
} Diagnostic_Charging_OverMCUTemp_Manager_t;


//****************NTC1过参数定义****************/
typedef enum {
    DIAGNOSTIC_CHG_NTC1TEMP_STATE_NORMAL = 0,    // 温度正常状态
    DIAGNOSTIC_CHG_NTC1TEMP_STATE_OVERHEATING,   // 过温检测中状态
    DIAGNOSTIC_CHG_NTC1TEMP_STATE_FAULT,        // 故障状态（过温报警）
    DIAGNOSTIC_CHG_NTC1TEMP_STATE_COOLING       // 冷却中状态
} Diagnostic_Charging_NTC1TempState;

typedef struct {
    Diagnostic_Charging_NTC1TempState currentState;
    float overheatThreshold;                   // 过温阈值 (°C)
    float cooldownThreshold;                   // 恢复阈值 (°C)
    uint32_t overheatDurationMs;               // 过温持续时间要求 (ms)
    uint32_t cooldownDurationMs;               // 冷却持续时间要求 (ms)
    uint32_t overheatCounterMs;                // 过温计时器
    uint32_t cooldownCounterMs;                // 冷却计时器
    bool isFaultActive;                       // 故障激活标志
} Diagnostic_Charging_OverNTC1Temp_Manager_t;



/****************************************接地异常****************************************/
// 接地状态定义
typedef enum {
    DIAGNOSTIC_CHG_GROUND_STATE_NORMAL = 0,         // 接地正常
    DIAGNOSTIC_CHG_GROUND_STATE_FAULT_DETECTING,    // 接地故障检测中
    DIAGNOSTIC_CHG_GROUND_STATE_FAULT_LOCKED,       // 接地故障锁定
    DIAGNOSTIC_CHG_GROUND_STATE_RECOVERING          // 故障恢复中
} Diagnostic_Charging_GroundState;

// 时间阈值定义（基于5ms定时器）
#define DIAGNOSTIC_CHARGING_GROUND_FAULT_TIME_THRESHOLD      200       // 1s = 200 * 5ms（故障检测时间）
#define DIAGNOSTIC_CHARGING_GROUND_RECOVERY_TIME_THRESHOLD   1000      // 5s = 1000 * 5ms（故障恢复时间）

typedef struct {
    Diagnostic_Charging_GroundState currentState;
    uint16_t faultTimerCount;           // 故障检测计时器
    uint16_t recoveryTimerCount;        // 故障恢复计时器
    bool isFaultLocked;                 // 故障是否锁定
    bool lastGroundStatus;              // 上次接地状态
    uint16_t faultTimeThreshold;         // 故障检测时间阈值（可设置）
    uint16_t recoveryTimeThreshold;     // 故障恢复时间阈值（可设置）
} Diagnostic_Charging_Ground_Manager_t;

// 充电自检管理器结构体
typedef struct {
    Diagnostic_Charging_LeakageCheck_Manager_t LeakageCheck;
    Diagnostic_Charging_ShortCircuit_Manager_t ShortCircuit;
    Diagnostic_Charging_Realy_Manager_t Relay;
    Diagnostic_Charging_Overcurrent_Manager_t Overcurrent;
    Diagnostic_Charging_Voltage_Manager_t Voltage;
    Diagnostic_Charging_OverMCUTemp_Manager_t OverMCUTemp;
    Diagnostic_Charging_OverNTC1Temp_Manager_t OverNTC1Temp;
    Diagnostic_Charging_Ground_Manager_t Ground;
} Diagnostic_Charging_Manager_t;

/****************************************放电诊断部分****************************************/
/****************************************漏电****************************************/
// 自检状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_LeakageCheck_IDLE = 0,      // 空闲状态
    DIAGNOSTIC_DISCHG_LeakageCheck_START,
} Diagnostic_Discharging_LeakageCheckState;

typedef struct {
    Diagnostic_Discharging_LeakageCheckState currentState;
    uint32_t count;
    bool Leakage_SCR_Status;
} Diagnostic_Discharging_LeakageCheck_Manager_t;

/****************************************继电器黏连****************************************/
// 继电器故障状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_RELAY_STATE_NORMAL = 0,      // 正常状态
    DIAGNOSTIC_DISCHG_RELAY_STATE_FAULT_DETECTING, // 故障检测中
    DIAGNOSTIC_DISCHG_RELAY_STATE_FAULT,           // 故障状态（粘连）
    DIAGNOSTIC_DISCHG_RELAY_STATE_RECOVERING      // 恢复中
} Diagnostic_Discharging_RelayFaultState;

// 继电器故障检测器结构体
typedef struct {
    Diagnostic_Discharging_RelayFaultState currentState;    // 当前状态
    uint32_t faultDurationMs;           // 故障确认持续时间(ms)
    uint32_t recoverDurationMs;         // 故障恢复持续时间(ms)
    uint32_t faultCounterMs;            // 故障计时器
    uint32_t recoverCounterMs;          // 恢复计时器
    bool isFaultActive;                 // 故障激活标志
    bool lastRelayStatus;               // 上一次继电器状态
} Diagnostic_Discharging_Realy_Manager_t;

/****************************************过流****************************************/
// 过流保护状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_OC_STATE_NORMAL = 0,        // 正常状态
    DIAGNOSTIC_DISCHG_OC_STATE_LEVEL1_DETECTING,   // 一段过流检测中
    DIAGNOSTIC_DISCHG_OC_STATE_LEVEL1_FAULT,        // 一段过流故障
    DIAGNOSTIC_DISCHG_OC_STATE_LEVEL2_FAULT,        // 二段过流故障
    DIAGNOSTIC_DISCHG_OC_STATE_LOCKED               // 流程锁死状态
} Diagnostic_Discharging_OvercurrentState;

// 恢复检测状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_OC_RECOVERY_IDLE = 0,        // 恢复空闲状态
    DIAGNOSTIC_DISCHG_OC_RECOVERY_CHECKING,        // 恢复条件检测中
    DIAGNOSTIC_DISCHG_OC_RECOVERY_CONDITION_MET,   // 恢复条件满足
    DIAGNOSTIC_DISCHG_OC_RECOVERY_COUNTDOWN        // 恢复倒计时中
} Diagnostic_Discharging_OvercurrentRecoveryState;

// 过流保护配置参数
typedef struct {
    float level1Threshold;              // 一段过流阈值(A)
    float level2Threshold;              // 二段过流阈值(A)
    uint32_t level1DurationMs;          // 一段过流持续时间(ms)
    uint32_t level2DurationMs;          // 二段过流响应时间(ms)
    uint32_t recoveryDelayMs;          // 恢复延时(ms)
    uint32_t recoveryCheckMs;          // 恢复检测时间(ms)
    uint8_t maxLevel1Faults;            // 最大一段过流次数
} Diagnostic_Discharging_OvercurrentConfig;

// 恢复检测参数
typedef struct {
    Diagnostic_Discharging_OvercurrentRecoveryState state;   // 恢复状态
    uint32_t checkCounterMs;             // 恢复检测计时器(ms)
    uint32_t countdownCounterMs;        // 恢复倒计时计时器(ms)
    bool conditionMet;                   // 恢复条件是否满足
    float currentThreshold;              // 恢复电流阈值(A)
} Diagnostic_Discharging_OvercurrentRecovery;

typedef struct {
    Diagnostic_Discharging_OvercurrentState currentState;
    Diagnostic_Discharging_OvercurrentConfig config;      // 配置参数
    Diagnostic_Discharging_OvercurrentRecovery recovery;    // 恢复检测器
    uint32_t level1CounterMs;           // 一段过流计时器
    uint32_t level2CounterMs;           // 二段过流计时器
    uint8_t level1FaultCount;           // 一段过流次数计数
    bool isFaultActive;                 // 故障激活标志
    bool isChargingLocked;              // 充电流程锁死标志
    float currentCurrent;               // 当前电流值(A)
} Diagnostic_Discharging_Overcurrent_Manager_t;

/****************************************过欠压****************************************/
// 过欠压保护状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_VOLTAGE_STATE_NORMAL = 0,        // 正常状态
    DIAGNOSTIC_DISCHG_VOLTAGE_STATE_OVER_ALARM,        // 过压报警状态
    DIAGNOSTIC_DISCHG_VOLTAGE_STATE_UNDER_ALARM,      // 欠压报警状态
    DIAGNOSTIC_DISCHG_VOLTAGE_STATE_OVER_RECOVERY,    // 过压恢复中状态
    DIAGNOSTIC_DISCHG_VOLTAGE_STATE_UNDER_RECOVERY,    // 欠压恢复中状态
    DIAGNOSTIC_DISCHG_VOLTAGE_STATE_LOCKED             // 过欠压故障锁死放电流程
} Diagnostic_Discharging_VoltageState;

// 电压状态检测枚举
typedef enum {
    DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_NORMAL = 0,
    DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_OVER,
    DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_UNDER,
    DIAGNOSTIC_DISCHG_VOLTAGE_STATUS_IN_RECOVERY_ZONE
} Diagnostic_Discharging_VoltageStatus;

// 电压阈值定义
#define DIAGNOSTIC_DISCHG_OVER_VOLTAGE_THRESHOLD       264000    // 过压阈值
#define DIAGNOSTIC_DISCHG_OVER_VOLTAGE_RECOVERY        254000    // 过压恢复阈值
#define DIAGNOSTIC_DISCHG_UNDER_VOLTAGE_THRESHOLD      176000    // 欠压阈值
#define DIAGNOSTIC_DISCHG_UNDER_VOLTAGE_RECOVERY       186000    // 欠压恢复阈值

// 时间阈值定义（基于5ms定时器）
#define DIAGNOSTIC_DISCHG_ABNORMAL_TIME_THRESHOLD      200       // 1s = 200 * 5ms（200次调用）
#define DIAGNOSTIC_DISCHG_RECOVERY_TIME_THRESHOLD      6000      // 30s = 6000 * 5ms（6000次调用）

typedef struct {
    Diagnostic_Discharging_VoltageState currentState;
    uint16_t timerCount;                // 通用计时器（5ms为单位）
    bool alarmStatus;                   // 报警状态
    uint16_t lastVoltage;               // 上次电压值
    uint8_t OvermaxFaults;              // 过压最大恢复次数
    uint8_t UndermaxFaults;             // 欠压最大恢复次数
    uint8_t OverFaultCount;             // 过压故障次数
    uint8_t UnderFaultCount;            // 欠压故障次数
} Diagnostic_Discharging_Voltage_Manager_t;

/****************************************MCU过温参数定义****************************************/
// 过温状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_MCUTEMP_STATE_NORMAL = 0,       // 温度正常状态
    DIAGNOSTIC_DISCHG_MCUTEMP_STATE_OVERHEATING,      // 过温检测中状态
    DIAGNOSTIC_DISCHG_MCUTEMP_STATE_FAULT,            // 故障状态（过温报警）
    DIAGNOSTIC_DISCHG_MCUTEMP_STATE_COOLING           // 冷却中状态
} Diagnostic_Discharging_MCUTempState;

typedef struct {
    Diagnostic_Discharging_MCUTempState currentState;
    float overheatThreshold;            // 过温阈值(℃)
    float cooldownThreshold;            // 恢复阈值(℃)
    uint32_t overheatDurationMs;        // 过温持续时间要求(ms)
    uint32_t cooldownDurationMs;       // 冷却持续时间要求(ms)
    uint32_t overheatCounterMs;         // 过温计时器
    uint32_t cooldownCounterMs;        // 冷却计时器
    bool isFaultActive;                 // 故障激活标志
} Diagnostic_Discharging_OverMCUTemp_Manager_t;

/****************************************NTC1过温参数定义****************************************/
// 过温状态定义
typedef enum {
    DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_NORMAL = 0,       // 温度正常状态
    DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_OVERHEATING,      // 过温检测中状态
    DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_FAULT,            // 故障状态（过温报警）
    DIAGNOSTIC_DISCHG_NTC1TEMP_STATE_COOLING           // 冷却中状态
} Diagnostic_Discharging_NTC1TempState;

typedef struct {
    Diagnostic_Discharging_NTC1TempState currentState;
    float overheatThreshold;            // 过温阈值(℃)
    float cooldownThreshold;            // 恢复阈值(℃)
    uint32_t overheatDurationMs;        // 过温持续时间要求(ms)
    uint32_t cooldownDurationMs;       // 冷却持续时间要求(ms)
    uint32_t overheatCounterMs;         // 过温计时器
    uint32_t cooldownCounterMs;        // 冷却计时器
    bool isFaultActive;                 // 故障激活标志
} Diagnostic_Discharging_OverNTC1Temp_Manager_t;

// 放电自检管理器结构体
typedef struct {
    Diagnostic_Discharging_LeakageCheck_Manager_t LeakageCheck;
    Diagnostic_Discharging_Realy_Manager_t Relay;
    Diagnostic_Discharging_Overcurrent_Manager_t Overcurrent;
    Diagnostic_Discharging_Voltage_Manager_t Voltage;
    Diagnostic_Discharging_OverMCUTemp_Manager_t OverMCUTemp;
    Diagnostic_Discharging_OverNTC1Temp_Manager_t OverNTC1Temp;
} Diagnostic_Discharging_Manager_t;

/****************************************充放电管理****************************************/
// 自检管理器结构体
typedef struct {
    Diagnostic_Charging_Manager_t charging;
    Diagnostic_Discharging_Manager_t discharging;
} Diagnostic_Manager_t;

extern Diagnostic_Manager_t Diagnostic_Manager;

/*函数声明*/
extern void APP_DIAG_CHG_Init(void);
extern void APP_DIAG_CHG_Update(void);
extern void APP_DIAG_CHG_CurReset(Diagnostic_Manager_t *Diagnostic);
extern void APP_Diag_ChargeManageMain_1ms(void);
extern void APP_DIAG_CHG_CurProtector_Set(Diagnostic_Manager_t *Diagnostic);


extern void APP_DIAG_DISCHG_Init(void);
extern void APP_DIAG_DISCHG_Update(void);
extern void APP_DIAG_DISCHG_CurReset(Diagnostic_Manager_t *Diagnostic);

#endif
