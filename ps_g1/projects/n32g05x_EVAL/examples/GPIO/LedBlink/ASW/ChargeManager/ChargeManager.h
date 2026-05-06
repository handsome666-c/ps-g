#ifndef _CHARGE_MANAGER_H
#define _CHARGE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "Charging.h"
#include "Selfcheck.h"
#include "Precheck.h"
#include "Diag.h"
#include "cp.h"
#include "Rly.h"
#include "Diodecheck.h"
#include "RTE_BSW.h"
#include "factory.h"

// 时间阈值定义（基于系统定时器，假设1ms或10ms定时器）
#define CHARGING_POWER_ON_STABLE_TIME_MS      3000    // 充电上电稳定等待时间3秒
#define DISCHARGING_POWER_ON_STABLE_TIME_MS   4000    // 放电上电稳定等待时间4秒
#define SELF_CHECK_TIME_MS                   2000    // 自检时间2秒

// 充电业务顶层状态定义
typedef enum {
    CHARGING_STATE_IDLE = 0,          // 空闲状态
    CHARGING_STATE_POWER_ON_WAIT,     // 充电上电等待系统稳定状态
    DISCHARGING_STATE_POWER_ON_WAIT,  // 放电上电等待系统稳定状态
    CHARGING_STATE_INIT,              // 充电应用初始化
    DISCHARGING_STATE_INIT,           // 放电应用初始化
    CHARGING_STATE_SELF_CHECK,        // 充电自检状态
    DISCHARGING_STATE_SELF_CHECK,     // 放电自检状态
    CHARGING_STATE_RUNNING,           // 充电运行功能状态
    DISCHARGING_STATE_RUNNING,        // 放电运行功能状态
    CHARGING_STATE_FAULT,             // 故障状态（可选）
    DISCHARGING_STATE_FAULT,          // 故障状态（可选）
    CALIBRATION_MODE,                 // 校准模式
    FACTORY_MODE                      // 工厂模式
} ChargingState;

enum {
    SYSSTATE_CHARGING = 0,            // 充电
    SYSSTATE_DISCHARGING,             // 放电
};

// 充电业务顶层状态机结构体
typedef struct {
    ChargingState currentState;       // 当前状态
    uint32_t stateTimer;              // 状态计时器（毫秒）
    uint32_t powerOnStableTime;       // 上电稳定时间阈值（可设置）
    uint32_t selfCheckTime;           // 自检时间阈值（可设置）
    bool selfCheckResult;             // 自检结果
} ChargingStateMachine;

extern ChargingStateMachine machine;
extern uint8_t DischgorchgSysState;

void APP_ChargeManagerInit(void);
void APP_ChargeManageMain(void);
void Enter_RunningChargingMode(void);
void Enter_RunningDischargingMode(void);
void chargingStateMachine_setcalibration(ChargingStateMachine *machine);
void chargingStateMachine_setfactory(ChargingStateMachine *machine);

uint8_t getSysState(void);

#endif
