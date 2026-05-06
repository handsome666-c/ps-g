#include "EcuM.h"

/**********************************************************************
*函数名称  : void EcuM_Init(void)
*参数      : void
*返回值    : void
*描述      : 驱动模块函数初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void EcuM_InitOne(void)
{
    /*WDG Init*/
    WdgM_Init();
    /*Port Init*/
    PortIf_Init();
    /*DIO Init*/
    DioIf_Init();
    /*ADC Init*/
    AdcIf_Init();
    /*PWM Init TIM2 RLY TIM3 led1 TIM4 led2 led3 TIM5 CP*/
    PwmIf_Init();
    /*FLS Init*/
    FlsIf_Init();
//    /*TMR Init 仅剩TIM1 */
//    TmrIf_Init();
    /*UART Init*/
//    UartIf_Init();

    WdgM_Trigger();
}

/**********************************************************************
*函数名称  : void EcuM_Init(void)
*参数      : void
*返回值    : void
*描述      : 应用层模块初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void EcuM_InitTwo(void)
{
    RTE_Call_ASW_Init();/*位置不要改动*/
    WdgM_Trigger();
}

/**********************************************************************
*函数名称  : void EcuM_InitThree(void)
*参数      : void
*返回值    : void
*描述      : 其他模块初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void EcuM_InitThree(void)
{
//    TmrIf_StartTimer();

    WdgM_Trigger();
}
