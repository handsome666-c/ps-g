#include "NVIC.h"

/**********************************************************************
*函数名称  : void NVIC_Configuration(IRQn_Type IRQn, uint8_t PRIORITY, FunctionalState cmd)
*参数      : IRQn(中断源)、PRIORITY(优先级)、cmd(ENABLE/DISABLE)
*返回值    : void
*描述      : DMA_CH1_2_IRQn中断使能
*编辑时间  :
*备注      : 初版
***********************************************************************/
void NVIC_Configuration(IRQn_Type IRQn, uint8_t PRIORITY, FunctionalState cmd)
{
    NVIC_InitType NVIC_InitStructure;

    /* Configure and enable ADC interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = cmd;
    NVIC_Init(&NVIC_InitStructure);
}
