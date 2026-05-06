#ifndef _CHARGING_H
#define _CHARGING_H

#include <stdint.h>
#include <stdlib.h>
#include "n32g05x.h"
#include "RTE_ASW.h"
#include "RTE_BSW.h"
#include "rtthread.h"
#include <string.h>
#include "Rly.h"
#include "PwmIf.h"
#include "cp.h"
#include "Precheck.h"
#include "Diodecheck.h"
#include "Diag.h"

/****************************************充电业务状态机模型创建+参数定义****************************************/
enum {
    E_ChargeEventRun_OK                 = 0,
    E_ChargeStateMachineRun_OK          = 1,
};

typedef struct {
    uint16_t topic;   // 事件类型
    uint16_t data;    // 事件参数
} charging_event_struct_t;

typedef enum {
    E_STATE_CHG_POST = 0,      // 开机自检
    E_STATE_CHG_IDLE,          // 空闲
    E_STATE_CHG_CONNECTED,     // 已插枪
    E_STATE_CHG_PRECHECK,      // 充电预检
    E_STATE_CHG_DIODECHECK,    // 二极管检测
    E_STATE_CHG_READY,         // 就绪
    E_STATE_CHG_CHARGING,      // 充电中
    E_STATE_CHG_VPAUSE,        // 车端暂停
    E_STATE_CHG_PPAUSE,        // 保护暂停
    E_STATE_CHG_WARNING,       // 告警
    E_STATE_CHG_FAULT,         // 故障
//  E_STATE_RECOVERY_LAST,    // 恢复到前一次状态
} charging_state_t;

typedef enum {
    E_EVENT_CHG_POST_SUCCESS_IDLE,        // 事件：开机自检成功-状态空闲 0
    E_EVENT_CHG_POST_SUCCESS_CONNECTED,   // 事件：开机自检成功-已插枪 1
    E_EVENT_CHG_POST_FAILURE,             // 事件：开机自检失败 2
    E_EVENT_CHG_PLUG_IN,                  // 事件：插枪 3
    E_EVENT_CHG_PLUG_OUT,                 // 事件：拔枪 4
    E_EVENT_CHG_NOAUTH_START,             // 事件：无需鉴权启动充电 5
    E_EVENT_CHG_PRECHECK_SUCCESS,         // 事件：充电预检成功 6
    E_EVENT_CHG_PRECHECK_FAILURE,         // 事件：充电预检失败 7
    E_EVENT_CHG_DIODECHECK_COMPLETE,      // 事件：二极管检测 8
    E_EVENT_CHG_DIODECHECK_FAILURE,       // 事件：二极管检测 9
    E_EVENT_CHG_VEHICLE_START,            // 事件：车机启动充电 10
    E_EVENT_CHG_VEHICLE_PAUSE,            // 事件：车机暂停 11
    E_EVENT_CHG_PROTECTION_PAUSE,         // 事件：保护暂停 12
    E_EVENT_CHG_PROTECTION_RECOVERY,      // 事件：保护恢复 13
    E_EVENT_CHG_VEHICLE_EXCESSIVE_START_COUNT, // 单次充电周期内车端过度启充（大于6次）14
    E_EVENT_CHG_WARNING,                  // 事件：告警 15
    E_EVENT_CHG_WARNING_RECOVERY,         // 事件：告警恢复 16
    E_EVENT_CHG_FAULT,                    // 事件：故障 17
    E_EVENT_CHG_FAULT_RECOVERY,          // 事件：故障恢复 18
    E_EVENT_CHG_SYSTEM_RESET,             // 事件：系统重启 19
} charging_event_t;

typedef void (*charging_state_action_fn)(const charging_event_struct_t*);

typedef struct {
    charging_state_t current_state;
    charging_event_t event;
    charging_state_t next_state;
    charging_state_action_fn action;
} charging_state_transition_t;

typedef struct {
    charging_state_t last_state;
    charging_state_t curr_state;
} charging_state_machine_params_t;

typedef struct {
    uint32_t errorCode;
} charging_info_t;


/*******************************充电-故障类型和恢复故障类型定义*******************************/
typedef enum {
    /* 故障优先级定义（从高到低） */
    E_CHARGING_FAULT_LEAKAGE = 0,           // 漏电 - bit0
    E_CHARGING_FAULT_SHORT_CIRCUIT,         // 短路 - bit1
    E_CHARGING_FAULT_RELAY_STICK,           // 继电器黏连 - bit2
    E_CHARGING_FAULT_OVER_CURRENT,          // 过流 - bit3
    E_CHARGING_FAULT_OVER_UNDER_VOLTAGE,    // 过欠压 - bit4
    E_CHARGING_FAULT_OVER_MCUTEMPERATURE,   // MCU过温 - bit5
    E_CHARGING_FAULT_OVER_NTC1TEMPERATURE,  // NTC1过温 - bit6
    E_CHARGING_FAULT_CP_ABNORMAL,           // CP异常 - bit7
    E_CHARGING_FAULT_DIODE_CHECK,           // 二极管检测 - bit8
    E_CHARGING_FAULT_GROUND_FAULT,          // 接地异常 - bit9
    E_CHARGING_FAULT_TYPE_MAX               // 故障类型总数
} fault_charging_type_t;

typedef struct {
    fault_charging_type_t type;
} event_fault_charging_data_t;

typedef enum {
    /* 故障优先级定义（从高到低） */
    E_CHARGING_FAULT_RECOVERY_LEAKAGE = 0,          // 漏电 - bit0
    E_CHARGING_FAULT_RECOVERY_CIRCUIT,              // 短路 - bit1
    E_CHARGING_FAULT_RECOVERY_STICK,                // 继电器黏连 - bit2
    E_CHARGING_FAULT_RECOVERY_CURRENT,              // 过流 - bit3
    E_CHARGING_FAULT_RECOVERY_OVER_UNDER_VOLTAGE,   // 过欠压 - bit4
    E_CHARGING_FAULT_RECOVERY_OVER_TEMPERATURE,     // 过温 - bit5
    E_CHARGING_FAULT_RECOVERY_CP_ABNORMAL,          // CP异常 - bit6
    E_CHARGING_FAULT_RECOVERY_DIODE_CHECK,          // 二极管检测 - bit7
    E_CHARGING_FAULT_RECOVERY_GROUND_FAULT,         // 接地异常 - bit8
} fault_recovery_charging_type_t;

typedef struct {
    fault_recovery_charging_type_t type;
} event_fault_recovery_charging_data_t;

/****************************************充电-保护暂停类型和保护暂停恢复类型定义****************************************/
typedef enum {
/* 保护暂停优先级定义（从高到低） */
    E_CHARGING_PROTECTION_PAUSE_LEAKAGE = 0,            // 漏电 - bit0
    E_CHARGING_PROTECTION_PAUSE_CIRCUIT,                // 短路 - bit1
    E_CHARGING_PROTECTION_PAUSE_STICK,                  // 继电器黏连 - bit2
    E_CHARGING_PROTECTION_PAUSE_CURRENT,                // 过流 - bit3
    E_CHARGING_PROTECTION_PAUSE_OVER_UNDER_VOLTAGE,     // 过欠压 - bit4
    E_CHARGING_PROTECTION_PAUSE_OVER_MCUTEMPERATURE,    // MCU过温 - bit5
    E_CHARGING_PROTECTION_PAUSE_OVER_NTC1TEMPERATURE,   // NTC1过温 - bit6
    E_CHARGING_PROTECTION_PAUSE_CP_ABNORMAL,            // CP异常 - bit7
    E_CHARGING_PROTECTION_PAUSE_DIODE_CHECK,            // 二极管检测 - bit8
    E_CHARGING_PROTECTION_PAUSE_GROUND_WARNING,          // 接地异常 - bit9
    E_CHARGING_PROTECTION_PAUSE_TYPE_MAX                // 故障类型总数
} protection_pause_charging_type_t;
typedef struct {
    protection_pause_charging_type_t type;
} event_protection_pause_charging_data_t;

typedef enum {
/* 保护暂停优先级定义（从高到低） */
    E_CHARGING_PROTECTION_RECOVERY_LEAKAGE = 0,            // 漏电 - bit0
    E_CHARGING_PROTECTION_RECOVERY_CIRCUIT,                // 短路 - bit1
    E_CHARGING_PROTECTION_RECOVERY_STICK,                  // 继电器黏连 - bit2
    E_CHARGING_PROTECTION_RECOVERY_CURRENT,                // 过流 - bit3
    E_CHARGING_PROTECTION_RECOVERY_OVER_UNDER_VOLTAGE,     // 过欠压 - bit4
    E_CHARGING_PROTECTION_RECOVERY_OVER_MCUTEMPERATURE,    // MCU过温 - bit5
    E_CHARGING_PROTECTION_RECOVERY_OVER_NTC1TEMPERATURE,   // NTC1过温 - bit6
    E_CHARGING_PROTECTION_RECOVERY_CP_ABNORMAL,            // CP异常 - bit7
    E_CHARGING_PROTECTION_RECOVERY_DIODE_CHECK,            // 二极管检测 - bit8
    E_CHARGING_PROTECTION_RECOVERY_GROUND_WARNING,         // 接地异常 - bit9
    E_CHARGING_PROTECTION_RECOVERY_TYPE_MAX                // 故障类型总数
} protection_recovery_charging_type_t;

typedef struct {
    protection_recovery_charging_type_t type;
} event_protection_recovery_charging_data_t;

/****************************************充电-告警故障类型和告警恢复故障类型定义****************************************/
typedef enum {
    // 告警类型
    E_CHARGING_WARNING_LEAKAGE = 0,          // 漏电 - bit0
    E_CHARGING_WARNING_SHORT_CIRCUIT,        // 短路 - bit1
    E_CHARGING_WARNING_RELAY_STICK,          // 继电器黏连 - bit2
    E_CHARGING_WARNING_OVER_CURRENT,         // 过流 - bit3
    E_CHARGING_WARNING_OVER_UNDER_VOLTAGE,   // 过欠压 - bit4
    E_CHARGING_WARNING_OVER_TEMPERATURE,     // 过温 - bit5
    E_CHARGING_WARNING_CP_ABNORMAL,          // CP异常 - bit6
    E_CHARGING_WARNING_DIODE_CHECK,          // 二极管检测 - bit7
    E_CHARGING_WARNING_GROUND_WARNING,       // 接地异常 - bit8
    E_CHARGING_WARNING_TYPE_MAX              // 故障类型总数
} warning_charging_type_t;

typedef struct {
    warning_charging_type_t type;
} event_warning_charging_data_t;

// 告警恢复类型定义
typedef enum {
    /* 故障优先级定义（从高到低） */
    E_CHARGING_WARNING_RECOVERY_LEAKAGE = 0,          // 漏电 - bit0
    E_CHARGING_WARNING_RECOVERY_SHORT_CIRCUIT,        // 短路 - bit1
    E_CHARGING_WARNING_RECOVERY_RELAY_STICK,          // 继电器黏连 - bit2
    E_CHARGING_WARNING_RECOVERY_OVER_CURRENT,         // 过流 - bit3
    E_CHARGING_WARNING_RECOVERY_OVER_UNDER_VOLTAGE,   // 过欠压 - bit4
    E_CHARGING_WARNING_RECOVERY_OVER_TEMPERATURE,     // 过温 - bit5
    E_CHARGING_WARNING_RECOVERY_CP_ABNORMAL,          // CP异常 - bit6
    E_CHARGING_WARNING_RECOVERY_DIODE_CHECK,          // 二极管检测 - bit7
    E_CHARGING_WARNING_RECOVERY_GROUND_WARNING,       // 接地异常 - bit8
    E_CHARGING_WARNING_RECOVERY_TYPE_MAX              // 故障类型总数
} warning_recovery_charging_type_t;

typedef struct {
    warning_recovery_charging_type_t type;
} event_warning_recovery_charging_data_t;

/****************************************二极管结果定义****************************************/
// 二极管检测结果
typedef enum {
    DIODE_RESULT_NORMAL = 0,    // 正常模式（有二极管）
    DIODE_RESULT_NO_DIODE       // 无二极管模式
} diode_result_t;

/****************************************车端启充次数****************************************/
// 充电特征配置
#define MAX_START_CHARGING_COUNT    6    // 单次充电最大启动次数
#define MAX_OVER_CURRENT_COUNT      3    // 单次充电最大过流次数

// 充电会话特征管理
typedef struct {
    uint16_t start_charging_count;      // 单次充电会话中启动充电的次数
    uint16_t over_current_count;        // 单次充电会话中过流次数
    bool is_charging_session_active;    // 充电会话是否活跃
} charging_session_t;

/****************************************放电业务状态机模型创建+参数定义****************************************/
typedef struct {
    uint16_t topic;    // 事件类型
    uint16_t data;     // 事件参数
} discharging_event_struct_t;

typedef enum {
    E_STATE_DISCHG_POST = 0,      // 开机自检
    E_STATE_DISCHG_PRECHECK,      // 放电预检
    E_STATE_DISCHG_DISCHARGING,   // 放电中
    E_STATE_DISCHG_PPAUSE,        // 保护暂停
    E_STATE_DISCHG_WARNING,       // 告警
    E_STATE_DISCHG_FAULT,         // 故障
} discharging_state_t;

typedef enum {
    E_EVENT_DISCHG_POST_SUCCESS = 0,    // 事件：开机自检成功-状态空闲 0
    E_EVENT_DISCHG_POST_FAILURE,       // 事件：开机自检失败
    E_EVENT_DISCHG_PRECHECK_SUCCESS,   // 事件：充电预检成功
    E_EVENT_DISCHG_PRECHECK_FAILURE,   // 事件：充电预检失败
    E_EVENT_DISCHG_PROTECTION_PAUSE,   // 事件：保护暂停
    E_EVENT_DISCHG_PROTECTION_RECOVERY,// 事件：保护恢复
    E_EVENT_DISCHG_WARNING,            // 事件：告警
    E_EVENT_DISCHG_WARNING_RECOVERY,  // 事件：告警恢复
    E_EVENT_DISCHG_FAULT,              // 事件：故障
    E_EVENT_DISCHG_FAULT_RECOVERY,    // 事件：故障恢复
    E_EVENT_DISCHG_SYSTEM_RESET,       // 事件：系统复位
} discharging_event_t;

typedef void (*discharging_state_action_fn)(const discharging_event_struct_t*);

typedef struct {
    discharging_state_t current_state;
    discharging_event_t event;
    discharging_state_t next_state;
    discharging_state_action_fn action;
} discharging_state_transition_t;

typedef struct {
    discharging_state_t last_state;
    discharging_state_t curr_state;
} discharging_state_machine_params_t;

typedef struct {
    uint32_t errorCode;
} discharging_info_t;

/****************************************放电-故障类型和恢复故障类型定义****************************************/
typedef enum {
    /* 故障优先级定义（从高到低） */
    E_DISCHARGING_FAULT_LEAKAGE = 0,            // 漏电
    E_DISCHARGING_FAULT_RELAY_STICK,            // 继电器黏连
    E_DISCHARGING_FAULT_OVER_CURRENT,           // 过流
    E_DISCHARGING_FAULT_OVER_UNDER_VOLTAGE,     // 过欠压
    E_DISCHARGING_FAULT_OVER_MCUTEMPERATURE,    // MCU过温
    E_DISCHARGING_FAULT_OVER_NTCTEMPERATURE,    // NTC1过温
    E_DISCHARGING_FAULT_TYPE_MAX                // 故障类型总数
} fault_discharging_type_t;

typedef struct {
    fault_discharging_type_t type;
} event_fault_discharging_data_t;

typedef enum {
    /* 故障优先级定义（从高到低） */
    E_DISCHARGING_RECOVERY_LEAKAGE = 0,            // 漏电
    E_DISCHARGING_RECOVERY_RELAY_STICK,            // 继电器黏连
    E_DISCHARGING_RECOVERY_OVER_CURRENT,           // 过流
    E_DISCHARGING_RECOVERY_OVER_UNDER_VOLTAGE,     // 过欠压
    E_DISCHARGING_RECOVERY_OVER_MCUTEMPERATURE,    // MCU过温
    E_DISCHARGING_RECOVERY_OVER_NTCTEMPERATURE,    // NTC1过温
    E_DISCHARGING_RECOVERY_TYPE_MAX                // 故障类型总数
} fault_recovery_discharging_type_t;

typedef struct {
    fault_recovery_discharging_type_t type;
} event_fault_recovery_discharging_data_t;

/****************************************放电-保护暂停类型和保护暂停恢复类型定义****************************************/
typedef enum {
    /* 保护暂停优先级定义（从高到低） */
    E_DISCHARGING_PROTECTION_PAUSE_LEAKAGE = 0,            // 漏电
    E_DISCHARGING_PROTECTION_PAUSE_STICK,                  // 继电器黏连
    E_DISCHARGING_PROTECTION_PAUSE_CURRENT,                // 过流
    E_DISCHARGING_PROTECTION_PAUSE_OVER_UNDER_VOLTAGE,     // 过欠压
    E_DISCHARGING_PROTECTION_PAUSE_OVER_MCUTEMPERATURE,    // MCU过温
    E_DISCHARGING_PROTECTION_PAUSE_OVER_NTCTEMPERATURE,    // NTC1过温
    E_DISCHARGING_PROTECTION_PAUSE_TYPE_MAX
} protection_pause_discharging_type_t;

typedef struct {
    protection_pause_discharging_type_t type;
} event_protection_pause_discharging_data_t;

typedef enum {
    /* 保护暂停恢复优先级定义（从高到低） */
    E_DISCHARGING_PROTECTION_RECOVERY_LEAKAGE = 0,            // 漏电
    E_DISCHARGING_PROTECTION_RECOVERY_STICK,                  // 继电器黏连
    E_DISCHARGING_PROTECTION_RECOVERY_CURRENT,                // 过流
    E_DISCHARGING_PROTECTION_RECOVERY_OVER_UNDER_VOLTAGE,     // 过欠压
    E_DISCHARGING_PROTECTION_RECOVERY_OVER_MCUTEMPERATURE,    // 过温
    E_DISCHARGING_PROTECTION_RECOVERY_OVER_NTCTEMPERATURE,    // 过温
    E_DISCHARGING_PROTECTION_RECOVERY_TYPE_MAX                // 故障类型总数
} protection_recovery_discharging_type_t;

typedef struct {
    protection_recovery_discharging_type_t type;
} event_protection_recovery_discharging_data_t;

/****************************************放电-告警故障类型和告警恢复故障类型定义****************************************/
typedef enum {
    // 告警类型
    E_DISCHARGING_WARNING_LEAKAGE = 0,          // 漏电 - bit0
    E_DISCHARGING_WARNING_SHORT_CIRCUIT,        // 短路 - bit1
    E_DISCHARGING_WARNING_RELAY_STICK,          // 继电器黏连 - bit2
    E_DISCHARGING_WARNING_OVER_CURRENT,         // 过流 - bit3
    E_DISCHARGING_WARNING_OVER_UNDER_VOLTAGE,   // 过欠压 - bit4
    E_DISCHARGING_WARNING_OVER_TEMPERATURE,     // 过温 - bit5
    E_DISCHARGING_WARNING_CP_ABNORMAL,          // CP异常 - bit6
    E_DISCHARGING_WARNING_DIODE_CHECK,          // 二极管检测 - bit7
    E_DISCHARGING_WARNING_GROUND_WARNING,       // 接地异常 - bit8
    E_DISCHARGING_WARNING_TYPE_MAX              // 故障类型总数
} warning_discharging_type_t;

typedef struct {
    warning_discharging_type_t type;
} event_warning_discharging_data_t;

// 告警恢复类型定义
typedef enum {
    /* 故障优先级定义（从高到低） */
    E_DISCHARGING_WARNING_RECOVERY_LEAKAGE = 0,          // 漏电 - bit0
    E_DISCHARGING_WARNING_RECOVERY_SHORT_CIRCUIT,        // 短路 - bit1
    E_DISCHARGING_WARNING_RECOVERY_RELAY_STICK,          // 继电器黏连 - bit2
    E_DISCHARGING_WARNING_RECOVERY_OVER_CURRENT,         // 过流 - bit3
    E_DISCHARGING_WARNING_RECOVERY_OVER_UNDER_VOLTAGE,   // 过欠压 - bit4
    E_DISCHARGING_WARNING_RECOVERY_OVER_TEMPERATURE,     // 过温 - bit5
    E_DISCHARGING_WARNING_RECOVERY_CP_ABNORMAL,          // CP异常 - bit6
    E_DISCHARGING_WARNING_RECOVERY_DIODE_CHECK,          // 二极管检测 - bit7
    E_DISCHARGING_WARNING_RECOVERY_GROUND_WARNING,       // 接地异常 - bit8
    E_DISCHARGING_WARNING_RECOVERY_TYPE_MAX              // 故障类型总数
} warning_recovery_discharging_type_t;

typedef struct {
    warning_recovery_discharging_type_t type;
} event_warning_recovery_discharging_data_t;

/****************************************函数声明****************************************/
// 充电函数声明
extern int APP_CHG_EventPubTopic(uint16_t topic);
extern int APP_CHG_EventPub(uint16_t topic, uint16_t data);
extern int APP_CHG_StateMachineDispatch(void);
extern void APP_CHG_StateMachineInit(void);
extern charging_state_t APP_CHG_GetChargeStatus(void);
extern uint8_t APP_CHG_GetStatusLED(void);
extern uint32_t APP_CHG_GetFault(void);
extern bool APP_CHG_IsFaultPresent(fault_charging_type_t faultType);
extern bool APP_CHG_IsAnyFaultPresent(void);
extern diode_result_t APP_CHG_GetDiodeStatus(void);

// 放电函数声明
extern int APP_DISCHG_EventPubTopic(uint16_t topic);
extern int APP_DISCHG_EventPub(uint16_t topic, uint16_t data);
extern int APP_DISCHG_StateMachineDispatch(void);
extern void APP_DISCHG_StateMachineInit(void);
extern discharging_state_t APP_DISCHG_GetChargeStatus(void);
extern uint8_t APP_DISCHG_GetStatusLED(void);
uint32_t APP_DISCHG_GetFault(void);
bool APP_DISCHG_IsFaultPresent(fault_discharging_type_t faultType);
bool APP_DISCHG_IsAnyFaultPresent(void);
#endif
