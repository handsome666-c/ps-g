#include "Pwmif.h"

uint8_t OC1 = 1;
uint8_t OC2 = 2;
uint8_t OC3 = 3;
uint8_t OC4 = 4;

uint16_t T5CCR3_Val = 0;  /*100%*/
uint16_t T4CCR1_Val = 1000; /*0%*/
uint16_t T3CCR1_Val = 1000; /*0%*/
uint16_t T3CCR2_Val = 1000; /*0%*/
uint16_t T2CCR2_Val = 0;    /*0%*/
//uint16_t T2CCR3_Val = 0; /*0%*/

static void PwmIf_InitCP(void);
static void PwmIf_InitRly(void);
static void PwmIf_InitLED1_G(void);
static void PwmIf_InitLED23_BR(void);

static void DisablePwmCP(void);
static void DisablePwmRly(void);
static void DisablePwmLED(void);

/**********************************************************************
*函数名称  : void PwmIf_Init(void)
*参数      : void
*返回值    : void
*描述      : PWM初始化函数
*编辑时间  :
*备注      : 初版
***********************************************************************/
void PwmIf_Init(void)
{
    PwmIf_InitCP();
    PwmIf_InitRly();
    PwmIf_InitLED1_G();
    PwmIf_InitLED23_BR();
}

/**********************************************************************
*函数名称  : void PwmIf_DeInit(void)
*参数      : void
*返回值    : void
*描述      : 反PWM初始化函数
*编辑时间  :
*备注      : 初版
***********************************************************************/
void PwmIf_DeInit(void)
{
    DisablePwmCP();
    DisablePwmRly();
    DisablePwmLED();
}

/**********************************************************************
*函数名称  : static void PwmIf_InitCP(void)
*参数      : void
*返回值    : void
*描述      : CP_PWM初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void PwmIf_InitCP(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;

    uint16_t PrescalerValue = 0;
    /* RCC configuration*/
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM5 | RCC_APB2_PERIPH_AFIO, ENABLE);

/*
TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
TIM3 Frequency = TIM3 counter clock/(AR + 1)
Pf = SystemCoreClock/(ADD+1) (PrescalerValue+1)
1KHZ = 64000000/(999+1)(63+1)
*/
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)(SystemCoreClock / 1000000) - 1;
    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period = 999;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CounterMode = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM5, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse = T5CCR3_Val;
    TIM_OCInitStructure.OCPolarity = TIM_OC_POLARITY_HIGH;

    TIM_InitOc3(TIM5, &TIM_OCInitStructure);
    TIM_ConfigOc3Preload(TIM5, TIM_OC_PRE_LOAD_ENABLE);

    TIM_ConfigArPreload(TIM5, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM5, ENABLE);

    TIM_EnableCtrlPwmOutputs(TIM5, ENABLE);
}

/**********************************************************************
*函数名称  : static void PwmIf_InitRly(void)
*参数      : void
*返回值    : void
*描述      : Rly_PWM初始化 20khz
*编辑时间  :
*备注      : 初版
***********************************************************************/
void PwmIf_InitRly(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;

    uint16_t PrescalerValue = 0;
    /* RCC configuration*/
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM2 | RCC_APB2_PERIPH_AFIO, ENABLE);

/*
TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
TIM3 Frequency = TIM3 counter clock/(AR + 1)
Pf = SystemCoreClock/(ADD+1) (PrescalerValue+1)
1KHZ = 64000000/(999+1)(63+1)
*/
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)(SystemCoreClock / 2000000) - 1;
    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period = 99;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CounterMode = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse = T2CCR2_Val;
    TIM_OCInitStructure.OCPolarity = TIM_OC_POLARITY_HIGH;

    TIM_InitOc2(TIM2, &TIM_OCInitStructure);
    TIM_ConfigOc2Preload(TIM2, TIM_OC_PRE_LOAD_ENABLE);

//  /* Output Compare Active Mode configuration: Channel2 */
//  TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
//  TIM_OCInitStructure.Pulse = T2CCR3_Val;
//  TIM_InitOc3(TIM2, &TIM_OCInitStructure);
//  TIM_ConfigOc3Preload(TIM2, TIM_OC_PRE_LOAD_ENABLE);

    TIM_ConfigArPreload(TIM2, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM2, ENABLE);

    TIM_EnableCtrlPwmOutputs(TIM2, ENABLE);
}

/**********************************************************************
*函数名称  : void PwmIf_InitLED1_G(void)
*参数      : void
*返回值    : void
*描述      : LED1_G初始化1khz
*编辑时间  :
*备注      : 初版
***********************************************************************/
void PwmIf_InitLED1_G(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;

    uint16_t PrescalerValue = 0;
    /* RCC configuration*/
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM4 | RCC_APB2_PERIPH_AFIO, ENABLE);

/*
TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
TIM3 Frequency = TIM3 counter clock/(AR + 1)
Pf = SystemCoreClock/(ADD+1) (PrescalerValue+1)
1KHZ = 64000000/(999+1)(63+1)
*/
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)(SystemCoreClock / 1000000) - 1;
    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period = 999;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CounterMode = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse = T4CCR1_Val;
    TIM_OCInitStructure.OCPolarity = TIM_OC_POLARITY_HIGH;

    TIM_InitOc1(TIM4, &TIM_OCInitStructure);
    TIM_ConfigOc1Preload(TIM4, TIM_OC_PRE_LOAD_ENABLE);

    TIM_ConfigArPreload(TIM4, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM4, ENABLE);

    TIM_EnableCtrlPwmOutputs(TIM4, ENABLE);
}

/**********************************************************************
*函数名称  : void PwmIf_InitLED23_BR(void)
*参数      : void
*返回值    : void
*描述      : LED23_BR初始化1khz
*编辑时间  :
*备注      : 初版
***********************************************************************/
void PwmIf_InitLED23_BR(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;

    uint16_t PrescalerValue = 0;
    /* RCC configuration*/
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM3 | RCC_APB2_PERIPH_AFIO, ENABLE);

/*
TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
TIM3 Frequency = TIM3 counter clock/(AR + 1)
Pf = SystemCoreClock/(ADD+1) (PrescalerValue+1)
1KHZ = 64000000/(999+1)(63+1)
*/
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)(SystemCoreClock / 1000000) - 1;
    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period = 999;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CounterMode = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse = T3CCR1_Val;
    TIM_OCInitStructure.OCPolarity = TIM_OC_POLARITY_HIGH;

    TIM_InitOc1(TIM3, &TIM_OCInitStructure);
    TIM_ConfigOc1Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);

    /* Output Compare Active Mode configuration: Channel2 */
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse = T3CCR2_Val;
    TIM_InitOc2(TIM3, &TIM_OCInitStructure);
    TIM_ConfigOc2Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);

    TIM_ConfigArPreload(TIM3, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM3, ENABLE);

    TIM_EnableCtrlPwmOutputs(TIM3, ENABLE);
}

/**********************************************************************
*函数名称  : void DisableCPPwm(void)
*参数      : void
*返回值    : void
*描述      : 反初始化继电器控制pwm函数
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DisablePwmCP(void)
{
    TIM_Enable(TIM5, DISABLE);
    TIM_EnableCtrlPwmOutputs(TIM5, DISABLE);
    PortIf_SetPinMode(g_Port_CP_ConfigTable[0]);
}

/**********************************************************************
*函数名称  : static void DisableLED1_GPwm(void)
*参数      : void
*返回值    : void
*描述      : 反初始化继电器控制pwm函数
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DisablePwmRly(void)
{
    TIM_Enable(TIM2, DISABLE);
    TIM_EnableCtrlPwmOutputs(TIM2, DISABLE);
    PortIf_SetPinMode(g_Port_Rly_ConfigTable[0]);
}

/**********************************************************************
*函数名称  : static void DisableLED1_GPwm(void)
*参数      : void
*返回值    : void
*描述      : 反初始化继电器控制pwm函数
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DisablePwmLED(void)
{
    TIM_Enable(TIM4, DISABLE);
    TIM_EnableCtrlPwmOutputs(TIM4, DISABLE);
    PortIf_SetPinMode(g_Port_LED_ConfigTable[0]);
    PortIf_SetPinMode(g_Port_LED_ConfigTable[1]);
    PortIf_SetPinMode(g_Port_LED_ConfigTable[2]);
}

/**********************************************************************
*函数名称  : void PwmIf_Set_CP_DutyCycleRly(TMR_T* tmr, uint16_t Duty)
*参数      : 设备索引: tmr, 占空比: uint16 Duty 0-1000
*返回值    : void
*描述      : 设置对应通道输出 PWM 占空比
*编辑时间  :
*备注      :
***********************************************************************/
void PwmIf_Set_Duty(TIM_Module* tmr, uint16_t OCy, uint16_t Duty)
{
    switch(OCy)
    {
        case 1: TIM_SetCmp1(tmr, Duty); break;
        case 2: TIM_SetCmp2(tmr, Duty); break;
        case 3: TIM_SetCmp3(tmr, Duty); break;
        case 4: TIM_SetCmp4(tmr, Duty); break;
        default: break;
    }
}
