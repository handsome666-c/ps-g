#ifndef _LED_H
#define _LED_H

#include "n32g05x.h"
#include <stdio.h>
#include <stdarg.h>
#include "RTE_BSW.h"
#include "charging.h"
#include "ChargeManager.h"
#include "log.h"

#define LED_LEDG (1)
#define LED_LEDB (2)
#define LED_LEDR (3)

typedef enum {
    LED_MODE_POST = 0,        // 开机自检
    LED_MODE_IDLE = 1,        // 空闲
    LED_MODE_CONN = 2,        // 插枪，连接
    LED_MODE_CHARGING = 3,    // 充电中
    LED_MODE_VPAUSE = 4,      // 结束或暂停
    LED_MODE_FAULT = 5,       // 充电故障

    LED_MODE_DISCHARGING_WAIT = 6,  // 放电等待
    LED_MODE_DISCHARGING = 7,       // 放电中
    LED_MODE_DISCHARGING_FAULT = 8  // 放电故障
} Led_OpType;

enum {
    LED_MODE_KEEP_ON = 0,
    LED_MODE_KEEP_OFF,
    LED_MODE_MODE_BLINK,   // 闪烁
    LED_MODE_MODE_BREATH,  // 呼吸
};

// LED工作状态枚举
typedef enum {
    LED_BLINKING,   // 闪烁中（执行n次闪烁）
    LED_PAUSING     // 暂停中（闪烁n次后停3秒）
} LedWorkState;

// 闪烁灯状态枚举
enum {
    BLINK_OFF,      // 关闭（常灭）
    BLINK_ON,       // 开启（常亮）
};

// 呼吸灯状态枚举（递增/递减亮度）
typedef enum {
    BREATH_INCREASE,    // 亮度递增（渐亮）
    BREATH_DECREASE     // 亮度递减（渐暗）
} BreathDir;

typedef struct {
    uint8_t mode;
    uint16_t time_10ms_cnt;
    LedWorkState blink_work_state;  // 当前工作状态

    // BlinkState blink_state;    // 当前状态（关闭/常亮/闪烁）
    uint8_t blink_cnt;              // 闪烁计数
    uint16_t blink_stop_cnt;        // 闪烁次数
    uint16_t blink_stop_time;       // 闪烁停止时间
    uint32_t blink_interval;        // 亮度变化的时间间隔（单位：ms）
    bool blink_current_level;       // 当前电平（true=高电平亮，false=低电平灭）

    uint16_t duty;
    uint16_t breath_min_duty;       // 最小占空比（0~period，对应最暗）
    uint16_t breath_max_duty;       // 最大占空比（0~period，对应最亮）
    uint16_t breath_step;           // 每次亮度变化的步长
    uint32_t breath_interval;       // 亮度变化的时间间隔（单位：ms）
    BreathDir breath_dir;           // 当前状态（递增/递减）
    uint16_t breath_current_duty;   // 当前占空比
} Led_mode_info_t;

typedef struct {
    uint8_t led_x;
    Led_mode_info_t mode;
} Led_info_t;

void APP_Led_Init(void);
void Led_MainFunction(void);

#endif
