#include "rly.h"

/* 继电器控制全局变量 */
RelayControl_t gRelayControl = {
    .state = RELAY_STATE_IDLE,
    .timer100ms = 0,
    .timer5ms = 0,
    .holdhightimer = 20,    // 20 * 5ms = 100ms
};

/**
 *\name     APP_RelayTask.
 *\fun      5ms任务周期调用，控制继电器
 *\param    none
 *\return   none
 */
void APP_RelayTask(void)  //后续锁定 删除整个函数
{
    /* 通用5ms计时器递增 */
    gRelayControl.timer5ms++;

    /* 100ms计时器递增（每20个5ms ticks） */
    if (gRelayControl.timer5ms % 20 == 0) {
        gRelayControl.timer100ms++;
    }

    /* 状态机处理 */
    switch (gRelayControl.state) {
        case RELAY_STATE_IDLE:
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_RELAY_ON, DIOIF_PIN_LOW);  //控制HW_DIO_CH_RELAY_ON输出低电平关闭
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_RELAY_OFF, DIOIF_PIN_HIGH); //控制HW_DIO_CH_RELAY_OFF输出高电平关闭
            break;
        case RELAY_STATE_HOLD_ON:
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT, DIOIF_PIN_LOW);   //断开短路检测IO
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_RELAY_ON, DIOIF_PIN_HIGH);
            /* 计时到达后切换到低电平状态 */
            if (gRelayControl.timer5ms >= gRelayControl.holdhightimer) { //100ms切换到低电平
                gRelayControl.state = RELAY_STATE_IDLE;
                gRelayControl.timer5ms = 0;
            }
            break;
        case RELAY_STATE_HOLD_OFF:
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT, DIOIF_PIN_LOW);   //断开短路检测IO
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_RELAY_OFF, DIOIF_PIN_LOW);
            /* 计时到达后切换到低电平状态 */
            if (gRelayControl.timer5ms >= gRelayControl.holdhightimer) { //100ms切换到低电平
                gRelayControl.state = RELAY_STATE_IDLE;
                gRelayControl.timer5ms = 0;
            }
            break;
        case RELAY_STATE_DELAY_HOLD_OFF:
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT, DIOIF_PIN_LOW); //断开短路检测IO
            /* 延时断开状态：每100ms检测一次电流 */
            if (gRelayControl.timer5ms % 20 == 0) {
                /* 条件：超时6秒，强制断开 */
                if (gRelayControl.timer100ms >= gRelayControl.dischargeTimeouts) {
                    gRelayControl.state = RELAY_STATE_HOLD_OFF;
                    gRelayControl.timer5ms = 0;
                    gRelayControl.timer100ms = 0;
                }
            }
            break;
        default:
            /* 异常状态处理：回到空闲状态 */
            gRelayControl.state = RELAY_STATE_IDLE;
            break;
    }
}

/**
 *\name     Relay hold on.
 *\fun      外部调用：启动继电器吸合
 *\param    none
 *\return   none
 */
void Relay_hold_on(void)
{
    gRelayControl.state = RELAY_STATE_HOLD_ON;
    gRelayControl.timer5ms = 0;
    gRelayControl.timer100ms = 0;
}

/**
 *\name     Relay hold off.
 *\fun      外部调用：立即断开继电器
 *\param    none
 *\return   none
 */
void Relay_hold_off(void)
{
    gRelayControl.state = RELAY_STATE_HOLD_OFF;
    gRelayControl.timer5ms = 0;
    gRelayControl.timer100ms = 0;
}

/**
 *\name     Relay delay hold off.
 *\fun      外部调用：延时断开继电器
 *\param    uint16_t ms(延时断开时间)
 *\return   none
 */
void Relay_delay_hold_off(uint16_t ms)
{
    gRelayControl.state = RELAY_STATE_DELAY_HOLD_OFF;
    gRelayControl.timer5ms = 0;
    gRelayControl.timer100ms = 0;
    gRelayControl.dischargeTimeouts = ms;
}
