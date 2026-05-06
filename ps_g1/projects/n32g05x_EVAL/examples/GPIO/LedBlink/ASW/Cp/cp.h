#ifndef _CP_H
#define _CP_H

#include "n32g05x.h"
#include "PwmIf.h"
#include "charging.h"
#include "RTE_BSW.h"

#define TYPE_2KW      0
#define TYPE_3_5KW    1

#define PRODUCT_TYPE  TYPE_2KW  // 产品类型：2+7KW、3.5+7KW

#define PRODUCT_MODE_POWER_HIGH 16
#define PRODUCT_MODE_POWER_LOW  8

#define VOLT_12V_MIN  1080u
#define VOLT_12V_MAX 1320u
#define VOLT_9V_MIN   780u
#define VOLT_9V_MAX  1020u
#define VOLT_6V_MIN   200u
#define VOLT_6V_MAX   720u

// 状态机控制结构
typedef struct {
    uint16_t timer;
    uint8_t Adapter_current;
    uint8_t Adapter_current_last;
} Adapter_state_machine_t;

typedef enum {
    E_CP_12V_DC = 0,    // 未连接 (12V直流)
    E_CP_9V_DC,         // 连接 (9V直流)
    E_CP_6V_DC,         // EVI准备就绪 (6V直流)
    E_CP_6V_PWM,        // 充电中 (6V PWM)
    E_CP_9V_PWM,        // 充电设备准备就绪 (9V PWM)
    E_CP_12V_PWM,       // 未连接 PWM (12V PWM)
    E_CP_UNKNOWN        // 未知状态
} cp_voltage_state_t;

typedef enum {
    E_STATE_UNPLUGGED = 0,  // 未连接
    E_STATE_PLUGGED,        // 车辆连接
    E_STATE_EV_READY,       // EV准备就绪
    E_STATE_CHARGING,       // 充电中
    E_STATE_EVSE_READY,     // 充电设备准备就绪-Electric Vehicle Supply Equipment
    E_STATE_ERROR           // 异常状态
} cp_state_t;

typedef enum {
    E_EVENT_NONE = 0,       //
    E_EVENT_PLUGIN,         // 插枪事件
    E_EVENT_PLUGOUT,        // 拔枪事件
    E_EVENT_EV_READY,       // EV准备就绪
    E_EVENT_EVSE_READY,     // 充电设备准备就绪
    E_EVENT_CHARGE_START,   // 启充事件
    E_EVENT_CHARGE_PAUSE,   // 暂停事件
    E_EVENT_CHARGE_STOP,    // 充电停止
    E_EVENT_ERROR,          // 错误事件
    E_EVENT_ERROR_RECOVERY, //错误恢复事件
} cp_event_t;

// 状态机控制结构
typedef struct {
    cp_state_t current_state;
    cp_event_t last_event;
    uint32_t state_timer;        // 状态持续时间(ms)
    uint8_t pwm_detected;        // PWM检测标志
    uint8_t pwm_active;
    bool is_in_fault_recovery;   // 新增：故障恢复标志
} cp_state_machine_t;


// 滤波器结构体修改
typedef struct {
    // 用于忽略窗口计数（替代时间戳）
    uint16_t ignore_counter; // 忽略窗口计数器，单位：采样次数
    float last_valid_voltage; // 上次的有效电压值
} cp_filter_t;

extern Adapter_state_machine_t st_Adapter_state_machine;

extern void Adapter_detection_current_Init(Adapter_state_machine_t *st_Adapter);

extern uint8_t Get_Adapter_detection_current(Adapter_state_machine_t *st_Adapter);

extern void APP_CP_Task(void);

extern void APP_CP_SetState(uint8_t pwm_active);

cp_state_t APP_CP_GetCurrentStatus(void);
cp_voltage_state_t APP_CP_DetectCPState(uint32_t voltage, uint8_t pwm_active);
#endif
