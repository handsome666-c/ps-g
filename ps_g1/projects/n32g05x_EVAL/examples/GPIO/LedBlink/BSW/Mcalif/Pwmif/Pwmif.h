#ifndef __PWMIF_H__
#define __PWMIF_H__

#include "Portif.h"
#include "Pwmif_Cfg.h"
#include "n32g05x.h"
#include "n32g05x_rcc.h"
#include "n32g05x_tim.h"
#include "n32g05x_gpio.h"

/* 外部引用配置表 */
extern uint8_t OC1;
extern uint8_t OC2;
extern uint8_t OC3;
extern uint8_t OC4;

/* 函数声明 */
extern void PwmIf_Init(void);
extern void PwmIf_DeInit(void);
extern void PwmIf_InitCP(void);
extern void PwmIf_InitRly(void);
extern void PwmIf_InitLED1_G(void);
extern void PwmIf_InitLED23_BR(void);
extern void DisablePwmCP(void);
extern void DisablePwmRly(void);
extern void DisablePwmLED(void);
extern void PwmIf_Set_Duty(TIM_Module* tmr, uint16_t OCy, uint16_t Duty);

#endif /* __PWMIF_H__ */
