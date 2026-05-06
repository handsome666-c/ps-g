#ifndef __LOG_H__
#define __LOG_H__

#ifndef LOG_ENABLE
#define LOG_ENABLE 1
#endif

#if LOG_ENABLE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "n32g05x.h"
#include "n32g05x_gpio.h"
#include "n32g05x_uart.h"
#include "n32g05x_rcc.h"
#include "NVIC.h"
#include "RTE_BSW.h"
#include "ChargeManager.h"
#include "Charging.h"
#include "AdcM.h"
#include "led.h"
#include "cp.h"
#include "Flashif.h"

#define LOG_NONE    0
#define LOG_ERROR   10
#define LOG_WARNING 20
#define LOG_INFO    30
#define LOG_DEBUG   40

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#if LOG_LEVEL >= LOG_INFO
#define log_info(...) printf(__VA_ARGS__)
#else
#define log_info(...)
#endif

#if LOG_LEVEL >= LOG_ERROR
#define log_error(...) printf(__VA_ARGS__)
#else
#define log_error(...)
#endif

#if LOG_LEVEL >= LOG_WARNING
#define log_warning(...) printf(__VA_ARGS__)
#else
#define log_warning(...)
#endif

#if LOG_LEVEL >= LOG_DEBUG
#define log_debug(...) printf(__VA_ARGS__)
#else
#define log_debug(...)
#endif

// 预检状态定义
typedef enum {
    CALIBRATION_ENTRY = 0,   // 空闲状态
    CALIBRATION_WAIT,        // 预检进行中
    CALIBRATION_INTO,        // 预检通过
    CALIBRATION_EXIT,        // 预检失败
} Calibration_State_t;

/* 自检子状态枚举（新增） */
typedef enum {
    CALIBRATION_SUBSTATE_IDLE,      // 子状态：空闲
    CALIBRATION_SUBSTATE_START,     // 子状态：开始当前测试项（进行初始化）
    CALIBRATION_SUBSTATE_CHECKING,  // 子状态：正在执行检测（周期性检查/消抖）
    CALIBRATION_SUBSTATE_WAITING    // 子状态：等待（可用于需要等待硬件响应的场景）
} Calibration_SubState_t;

typedef struct {
    Calibration_State_t state;      // 状态
    Calibration_SubState_t subState; // 子状态
    uint32_t timer;
    uint32_t readBuffer[2];
} calibration_Manager_t;

extern calibration_Manager_t calibration_Manager;

void log_init(void);
void Log_Printf_Main(void);
extern uint8_t get_calibration_mode_status(calibration_Manager_t *calibration_Manager);
extern void Calibration_Update(calibration_Manager_t *calibration_Manager);

#else /* !LOG_ENABLE */

#define log_info(...)
#define log_warning(...)
#define log_error(...)
#define log_debug(...)
#define log_init()

#endif

#define log_func() log_debug("call %s\r\n", __FUNCTION__)

#endif /* __LOG_H__ */
