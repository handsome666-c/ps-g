#ifndef _PRECHECK_H
#define _PRECHECK_H

#include "n32g05x.h"
#include "RTE_BSW.h"
#include <stdbool.h>
#include "Charging.h"
#include "Selfcheck.h"

// 预检状态定义
typedef enum {
    PRECHECK_IDLE = 0,      // 空闲状态
    PRECHECK_RUNNING,       // 预检进行中
    PRECHECK_PASSED,        // 预检通过
    PRECHECK_FAILED,        // 预检失败
    PRECHECK_WAITING        // 预检等待
} PreCheck_State_t;

/* 自检子状态枚举（新增） */
typedef enum {
    PRECHECK_SUBSTATE_IDLE,      // 子状态：空闲
    PRECHECK_SUBSTATE_START,     // 子状态：开始当前测试项（进行初始化）
    PRECHECK_SUBSTATE_CHECKING,  // 子状态：正在执行检测（周期性检查/消抖）
    PRECHECK_SUBSTATE_WAITING    // 子状态：等待（可用于需要等待硬件响应的场景）
} PreCheck_SubState_t;

/*************************Charging_precheck*************************/
/* 自检测试项故障码枚举 */
typedef enum {
    PRECHECK_ERROR_CHARGING_NONE = 0,
    PRECHECK_ERROR_CHARGING_LEAKAGE,   // 漏电故障
} PreCheck_Error_charging_t;

/* 自检测试项故障码结构体 */
typedef struct {
    PreCheck_Error_charging_t errorcode;
} PreCheck_Charging_Error_t;

/* 预检测试项索引枚举 */
typedef enum {
    PRECHECK_INDEX_CHARGING_LEAKAGE = 0,   // 漏电检测
    PRECHECK_INDEX_CHARGING_TEST_MAX       // 测试项总数
} PreCheck_charging_currentTestIndex_t;

/* 自检测试项索引结构体 */
typedef struct {
    PreCheck_charging_currentTestIndex_t currentTestIndex;
} PreCheck_Charging_Index_t;

// 预检管理器结构体
typedef struct {
    PreCheck_State_t state;               // 状态
    PreCheck_SubState_t subState;         // 子状态
    PreCheck_Charging_Index_t index;
    PreCheck_Charging_Error_t error;
    uint32_t timer;
    uint8_t S_out_count;
    bool testResults[PRECHECK_INDEX_CHARGING_TEST_MAX];  // 测试项目的结果
    bool faultStatus[PRECHECK_INDEX_CHARGING_TEST_MAX]; // 故障状态
} PreCheck_Charging_Manager_t;

/*************************Discharging_precheck*************************/
#define PRECHECK_DISCHG_RELAYCHK_THRESHOLD  10000   // 继电器粘连检测电压阈值10V

/* 自检测试项故障码枚举 */
typedef enum {
    PRECHECK_ERROR_DISCHARING_NONE = 0,
    PRECHECK_ERROR_DISCHARING_RELAY_STICK,  // 继电器粘连故障
} PreCheck_discharging_Error_Code_t;

/* 自检测试项故障码结构体 */
typedef struct {
    PreCheck_discharging_Error_Code_t errorcode;
} PreCheck_Discharging_Error_t;

/* 预检测试项索引枚举 */
typedef enum {
    PRECHECK_INDEX_DISCHARING_RELAY_STICK = 0,  // 继电器粘连检测
    PRECHECK_INDEX_DISCHARING_TEST_MAX          // 测试项总数
} PreCheck_discharging_currentTestIndex_t;

/* 自检测试项索引结构体 */
typedef struct {
    PreCheck_discharging_currentTestIndex_t currentTestIndex;
} PreCheck_Discharging_Index_t;

// 预检管理器结构体
typedef struct {
    PreCheck_State_t state;               // 状态
    PreCheck_SubState_t subState;         // 子状态
    PreCheck_Discharging_Index_t index;
    PreCheck_Discharging_Error_t error;
    uint32_t timer;
    bool Leakage_Cali_Flag; // 漏电校准标志
    bool Leakage_Check_Flag; // 漏电自检标志
    bool testResults[PRECHECK_INDEX_DISCHARING_TEST_MAX];  // 测试项目的结果
    bool faultStatus[PRECHECK_INDEX_DISCHARING_TEST_MAX]; // 故障状态
} PreCheck_Discharging_Manager_t;

// 自检管理器结构体
typedef struct {
    PreCheck_Charging_Manager_t charging;
    PreCheck_Discharging_Manager_t discharging;
} PreCheck_Manager_t;

extern PreCheck_Manager_t Pre_Manager;

void PreCheck_Charging_Init(PreCheck_Manager_t *PreCheckManager);
void PreCheck_Charging_Start(PreCheck_Manager_t *PreCheckManager);
void PreCheck_Charging_Update(PreCheck_Manager_t *PreCheckManager);

void PreCheck_Discharging_Init(PreCheck_Manager_t *PreCheckManager);
void PreCheck_Discharging_Start(PreCheck_Manager_t *PreCheckManager);
void PreCheck_Discharging_Update(PreCheck_Manager_t *PreCheckManager);

#endif
