#ifndef _RLY_H
#define _RLY_H

#include <stdint.h>
#include <stdbool.h>
#include "n32g05x.h"
#include "RTE_BSW.h"
#include <stdint.h>
#include "PwmIf.h"
#include "DioIf.h"
#include "Dioif_Cfg.h"

/* 状态机状态定义 */
typedef enum {
    RELAY_STATE_IDLE = 0,           // 空闲状态
    RELAY_STATE_HOLD_ON,            // relay_on high
    RELAY_STATE_HOLD_OFF,           // relay_off high
    RELAY_STATE_DELAY_HOLD_OFF      // delay relay off high
} RelayState_t;

/* 继电器控制结构体 */
typedef struct {
    RelayState_t state;             // 当前状态
    uint32_t timer100ms;            // 100ms计时器（用于电流检测）
    uint32_t timer5ms;              // 通用5ms计时器
    uint16_t holdhightimer;         // 高电平保持时间（单位：5ms ticks）
    uint16_t dischargeTimeouts;    // 延时断开继电器时间
} RelayControl_t;

/* 外部变量和函数声明 */
void APP_RelayTask(void);
void Relay_hold_on(void);
void Relay_hold_off(void);
void Relay_delay_hold_off(uint16_t ms);

#endif
