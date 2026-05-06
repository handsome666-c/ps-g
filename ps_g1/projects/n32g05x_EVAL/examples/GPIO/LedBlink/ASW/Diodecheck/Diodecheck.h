#ifndef _DIODECHECK_H
#define _DIODECHECK_H

#include "n32g05x.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "charging.h"
#include "RTE_BSW.h"

// 二极管检测状态定义
typedef enum {
    DIODECHECK_IDLE = 0,      // 空闲状态
    DIODECHECK_RUNNING,       // 预检进行中
    DIODECHECK_PASSED,        // 预检通过
    DIODECHECK_FAILED         // 预检失败
} DiodeCheck_State_t;

/* 自检子状态枚举（新增） */
typedef enum {
    DIODECHECK_SUBSTATE_IDLE,      // 子状态：空闲
    DIODECHECK_SUBSTATE_START,     // 子状态：开始当前测试项（进行初始化）
    DIODECHECK_SUBSTATE_CHECKING,  // 子状态：正在执行检测（周期性检查/消抖）
    DIODECHECK_SUBSTATE_WAITING    // 子状态：等待（可用于需要等待硬件响应的场景）
} DiodeCheck_SubState_t;

// 错误码定义
typedef enum {
    DIODECHECK_ERROR_NONE = 0,
    DIODECHECK_ERROR,
} DiodeCheck_Error_t;

/* 预检测试项索引枚举 */
typedef enum {
    DIODECHECK_INDEX = 0,    // 二极管检测
    DIODECHECK_INDEX_MAX     // 测试项总数
} DiodeCheck_Index_t;

typedef enum {
    DIODECHECK_DURATION = 200,
} DiodeCheck_Duration_t;

typedef enum {
    DIODECHECK_DEBOUNCE_THRESHOLD = 30,
} DiodeCheck_Debounce_Threshold_t;

// 预检管理器结构体
typedef struct {
    DiodeCheck_State_t state;
    uint32_t timer;
    uint32_t errorCode;
    bool testResults[DIODECHECK_INDEX_MAX];    // 测试项目的结果
    uint8_t currentTestIndex;
    DiodeCheck_SubState_t subState;           // 子状态（新增）
    bool faultStatus[DIODECHECK_INDEX_MAX];   // 故障状态
} DiodeCheck_Manager_t;

static DiodeCheck_Manager_t diodeCheckManager;

void APP_DiodeCheck_Init(void);
void APP_DiodeCheck_Start(void);
void APP_DiodeCheck_Update(void);

#endif
