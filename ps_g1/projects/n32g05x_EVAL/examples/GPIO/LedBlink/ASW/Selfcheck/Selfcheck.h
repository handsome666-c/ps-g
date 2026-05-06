#ifndef _SELFCHECK_H
#define _SELFCHECK_H

#include "n32g05x.h"
#include "RTE_BSW.h"
#include "log.h"
#include "Portif.h"
#include "cp.h"
#include "rly.h"
#include "Charging.h"
#include "ChargeManager.h"

// 自检状态定义
typedef enum {
    SELF_TEST_IDLE = 0,      // 空闲状态
    SELF_TEST_RUNNING,       // 自检进行中
    SELF_TEST_PASSED,        // 自检通过
    SELF_TEST_FAILED         // 自检失败
} SelfTest_State_t;

/* 自检子状态枚举（新增） */
typedef enum {
    SUBSTATE_IDLE,           // 子状态：空闲
    SUBSTATE_START,          // 子状态：开始当前测试项（进行初始化）
    SUBSTATE_WAITING,        // 子状态：等待（可用于需要等待硬件响应的场景）
    SUBSTATE_CHECKING        // 子状态：正在执行检测（周期性检查/消抖）
} SelfTest_SubState_t;

/*************************Charging_selfTest*************************/
// 漏电阈值定义
#define LEAKAGE_TIMEOUT_MS         300    // 漏电超时时间      // 瑞慈: 200ms 盛象: A_250ms B_280ms
#define LEAKAGE_HIGH_LEVEL_DETECT_MS 1600 // 漏电高电平检测时间 // 瑞慈: 1400ms 盛象: A_1300ms B_1600ms
#define LEAKAGE_LOW_LEVEL_DETECT_MS  1700 // 漏电低电平检测时间 // 瑞慈: 1500ms 盛象: A_1400ms B_1700ms

// 电压阈值定义
#define SELFCHECK_CHG_OVER_VOLTAGE_THRESHOLD    275000 // 充电过压阈值275V
#define SELFCHECK_CHG_UNDER_VOLTAGE_THRESHOLD   80000  // 充电欠压阈值80V

/* 自检测试项故障码枚举 */
typedef enum {
    ERROR_CHARGING_NONE = 0,
    ERROR_CHARGING_PGD_FAULT,             // 接地保护故障
    ERROR_CHARGING_RELAY_STICK_FAULT,     // 继电器黏连
    ERROR_CHARGING_LEAKAGE_FAULT,         // 漏电
    ERROR_CHARGING_SHORTCIRCUIT_FAULT,    // 短路
    ERROR_CHARGING_OVERVOLTAGE,           // 过压
    ERROR_CHARGING_UNDERVOLTAGE,          // 欠压
} Error_charging_Code_t;

/* 自检测试项故障码结构体 */
typedef struct {
    Error_charging_Code_t errorcode;
} SelfTest_Charging_Error_t;

/* 自检测试项索引枚举 */
typedef enum {
//  Selfcheck_INDEX_CHARGING_PGD_CHECK = 0, // 接地保护检测, 建议开机自检不做
    Selfcheck_INDEX_CHARGING_RELAY_STICK = 0,  // 继电器黏连检测
    Selfcheck_INDEX_CHARGING_LEAKAGE,          // 漏电检测
    Selfcheck_INDEX_CHARGING_SHORTCIRCUIT,     // 短路检测
    Selfcheck_INDEX_CHARGING_OVERVOLTAGE,      // 过压检测
    Selfcheck_INDEX_CHARGING_UNDERVOLTAGE,    // 欠压检测
    Selfcheck_INDEX_CHARGING_TEST_MAX          // 测试项总数
} SelfTest_charging_currentTestIndex_t;

/* 自检测试项索引结构体 */
typedef struct {
    SelfTest_charging_currentTestIndex_t currentTestIndex;
} SelfTest_Charging_Index_t;

/*************************Discharging_selfTest*************************/
// 电压阈值定义
#define SELFCHECK_DISCHG_OVER_VOLTAGE_THRESHOLD    264000 // 放电过压阈值264V
#define SELFCHECK_DISCHG_UNDER_VOLTAGE_THRESHOLD   176000 // 放电欠压阈值176V

/* 自检测试项故障码枚举 */
typedef enum {
    ERROR_DISCHARGING_NONE = 0,
    ERROR_DISCHARGING_RELAY_STICK_FAULT,     // 继电器黏连
    ERROR_DISCHARGING_LEAKAGE_FAULT,         // 漏电
    ERROR_DISCHARGING_OVERVOLTAGE,           // 过压
    ERROR_DISCHARGING_UNDERVOLTAGE,         // 欠压
} Error_discharging_Code_t;

/* 自检测试项故障码结构体 */
typedef struct {
    Error_discharging_Code_t errorcode;
} SelfTest_Discharging_Error_t;

/* 自检测试项索引枚举 */
typedef enum {
    Selfcheck_INDEX_DISCHARGING_LEAKAGE = 0, // 漏电检测
    Selfcheck_INDEX_DISCHARGING_TEST_MAX     // 测试项总数
} SelfTest_discharging_currentTestIndex_t;

/* 自检测试项索引结构体 */
typedef struct {
    SelfTest_discharging_currentTestIndex_t currentTestIndex;
} SelfTest_Discharging_Index_t;

// 充电自检管理器结构体
typedef struct {
    SelfTest_State_t state;                  // 状态
    SelfTest_SubState_t subState;            // 子状态
    SelfTest_Charging_Index_t index;
    SelfTest_Charging_Error_t error;
    uint32_t timer;
    uint32_t timer100ms;
    uint8_t S_out_count;
    bool testResults[Selfcheck_INDEX_CHARGING_TEST_MAX]; // 测试项目的结果
    bool faultStatus[Selfcheck_INDEX_CHARGING_TEST_MAX];  // 故障状态
} SelfTest_Charging_Manager_t;

// 放电自检管理器结构体
typedef struct {
    SelfTest_State_t state;                  // 状态
    SelfTest_SubState_t subState;            // 子状态
    SelfTest_Discharging_Index_t index;
    SelfTest_Discharging_Error_t error;
    uint32_t timer;
    uint32_t timer100ms;
    uint8_t S_out_count;
    bool testResults[Selfcheck_INDEX_DISCHARGING_TEST_MAX]; // 测试项目的结果
    bool faultStatus[Selfcheck_INDEX_DISCHARGING_TEST_MAX];  // 故障状态
} SelfTest_Discharging_Manager_t;

// 自检管理器结构体
typedef struct {
    SelfTest_Charging_Manager_t charging;
    SelfTest_Discharging_Manager_t discharging;
} SelfTest_Manager_t;

extern SelfTest_Manager_t Self_Manager;

/*************************函数声明*************************/
void SelfTest_Charging_Init(SelfTest_Manager_t *selfTestManager);
void SelfTest_Charging_Start(SelfTest_Manager_t *selfTestManager);
void SelfTest_Charging_Update(SelfTest_Manager_t *selfTestManager);

void SelfTest_Discharging_Init(SelfTest_Manager_t *selfTestManager);
void SelfTest_Discharging_Start(SelfTest_Manager_t *selfTestManager);
void SelfTest_Discharging_Update(SelfTest_Manager_t *selfTestManager);

#endif
