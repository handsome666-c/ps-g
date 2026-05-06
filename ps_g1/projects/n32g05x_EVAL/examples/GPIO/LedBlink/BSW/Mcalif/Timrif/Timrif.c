#include "n32g05x.h"
#include "Timrif.h"
#include "rtthread.h"

uint32_t TmrIf_systick_cnt = 0;

__IO uint16_t CCR1_Val = 500;

/**********************************************************************
*函数名称  : void RCC_TIM_Configuration(void)
*参数      : void
*返回值    : void
*描述      : Clock
*编辑时间  :
*备注      : 初版
***********************************************************************/
static void RCC_TIM_Configuration(void)
{
    /* PCLK1 = HCLK/4 */
    // RCC_ConfigPclk1(RCC_HCLK_DIV4);

    /* TIMx clocks enable */
    // RCC_EnableAPB1PeriphClk(RCC_APB2_PERIPH_TIM3, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM2, ENABLE);
}

/**********************************************************************
*函数名称  : void TmrIf_Init(void)
*参数      : void
*返回值    : void
*描述      : TMR初始化
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void TmrIf_Init(void)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;

    uint16_t PrescalerValue = 0;

    /* System Clocks Configuration */
    RCC_TIM_Configuration();

    /* NVIC Configuration */
    NVIC_Configuration(TIM2_IRQn, NVIC_PRIORITY_1, ENABLE);

    /* GPIO Configuration */
    // GPIO_Configuration();

    /*
    TIM2 Configuration: Output Compare Timing Mode:
    TIM2 counter clock at 6 MHz
    CC1 update rate = TIM2 counter clock / CCR1_Val
    CC2 update rate = TIM2 counter clock / CCR2_Val
    CC3 update rate = TIM2 counter clock / CCR3_Val
    CC4 update rate = TIM2 counter clock / CCR4_Val
    */
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)((SystemCoreClock) / 1000000) - 1;

    /* Time base configuration */
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period    = 999;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv    = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CounterMode = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);

    /* Prescaler configuration */
    TIM_ConfigPrescaler(TIM2, PrescalerValue, TIM_PSC_RELOAD_MODE_IMMEDIATE);

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_OCInitStructure.OCMode       = TIM_OCMODE_TIMING;
    TIM_OCInitStructure.OutputState  = TIM_OUTPUT_STATE_DISABLE;
    TIM_OCInitStructure.Pulse        = CCR1_Val;
    TIM_OCInitStructure.OCPolarity    = TIM_OC_POLARITY_HIGH;

    TIM_InitOc1(TIM2, &TIM_OCInitStructure);

#if 0
    /* TIM IT enable */
    TIM_ConfigInt(TIM2, TIM_INT_UPDATE, ENABLE);

    /* TIM2 enable counter */
    TIM_Enable(TIM2, ENABLE);
#endif
}

/**********************************************************************
*函数名称  : void TmrIf_StartTimer(void)
*参数      : void
*返回值    : void
*描述      : TMR start
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void TmrIf_StartTimer(void)
{
    /* TIM IT enable */
    TIM_ConfigInt(TIM2, TIM_INT_UPDATE, ENABLE);

    /* TIM2 enable counter */
    TIM_Enable(TIM2, ENABLE);

// #if(SMM_EN_CLASS == SMM_ENABLE)
// TIM_Enable(TIM4);
// #endif
    // SysTick_Config(SystemCoreClock / 1000);/*system tick init 1ms*/
}

/**********************************************************************
*函数名称  : void TmrIf_DeInit(void)
*参数      : void
*返回值    : void
*描述      : TMR反初始化
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void TmrIf_DeInit(void)
{

}

/**********************************************************************
*函数名称  : void TmrIf_DelayMs(uint32_t ms)
*参数      : uint32_t ms
*返回值    : void
*描述      : 延时函数, ms
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void TmrIf_DelayMs(uint32_t ms)
{
    volatile uint32_t delay = ms;
    TmrIf_systick_cnt = 0;
    while (TmrIf_systick_cnt <= delay)
    {
#if(WDGM_EN_CLASS == WDGM_ENABLE)
        WdgM_Trigger();
#endif
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetFlagStatus(TIM2, TIM_FLAG_UPDATE) != RESET)
    {
        TIM_ClearFlag(TIM2, TIM_FLAG_UPDATE);

        TmrIf_systick_cnt += 1;

//        if(TmrIf_systick_cnt >= 1000)
//        {
//            rt_kprintf(">>> tim thread start!\n");
//            TmrIf_systick_cnt = 0;
//        }
    }
}
