#include "Portif.h"

bool S_out_Flag = false;

static void PortIf_PortEXTI12_Enable(void);

/**********************************************************************
*函数名称  : void PortIf_Init(void)
*参数      : void
*返回值    : void
*描述      : Port初始化
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void PortIf_Init(void)
{
    PortIfCfg_PinDrv_Init(g_PortInitConfigTable);

    PortIf_PortEXTI12_Enable();
}

/**********************************************************************
*函数名称  : void PortIf_SetPinMode(const Port_settings_config_t ConfigPtr)
*参数      : config引脚配置
*返回值    : void
*描述      : 设置引脚模式
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void PortIf_SetPinMode(const Port_settings_config_t ConfigPtr)
{
    GPIO_InitType GPIO_InitStructure;
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = ConfigPtr.Pin;
    GPIO_InitStructure.GPIO_Mode = ConfigPtr.Mode;
    GPIO_InitStructure.GPIO_Pull = ConfigPtr.Pull;
    GPIO_InitStructure.GPIO_Slew_Rate = ConfigPtr.Slew_rate;
    GPIO_InitStructure.GPIO_Current = ConfigPtr.Current;
    GPIO_InitStructure.GPIO_Alternate = ConfigPtr.Alternate;
    GPIO_InitPeripheral(ConfigPtr.Port, &GPIO_InitStructure);
}

/**********************************************************************
*函数名称  : static void PortIf_PortInt2_Enable(void)
*参数      : void
*返回值    : void
*描述      : PC5中断初始化
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
static void PortIf_PortEXTI12_Enable(void)
{
    EXTI_InitType EXTI_InitStructure;

    /*Configure key EXTI Line to key input Pin*/
    GPIO_ConfigEXTILine(GPIOD_PORT_SOURCE, GPIO_PIN_SOURCE12);
    /*Configure EXTI line*/
    EXTI_InitStructure.EXTI_Line = EXTI_LINE12;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);;

    NVIC_Configuration(EXTI4_15_IRQn, NVIC_PRIORITY_3, ENABLE);
}

/**********************************************************************
*函数名称  : void PortIf_PortEXTI_Enable(IRQn_Type IRQn, uint8_t PRIORITY)
*参数      : IRQn:中断向量, PRIORITY:优先级
*返回值    : void
*描述      : Port中断使能
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void PortIf_PortEXTI_Enable(IRQn_Type IRQn, uint8_t PRIORITY)
{
    NVIC_Configuration(IRQn, PRIORITY, ENABLE);
}

/**********************************************************************
*函数名称  : void PortIf_PortEXTI_Disable(IRQn_Type IRQn)
*参数      : IRQn:中断向量,
*返回值    : void
*描述      : Port中断禁止
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void PortIf_PortEXTI_Disable(IRQn_Type IRQn)
{
    NVIC_Configuration(IRQn, 1, DISABLE);
}

/**********************************************************************
*函数名称  : void EXTI4_15_IRQHandler(void)
*参数      : void
*返回值    : void
*描述      : Port15EXTI中断服务函数
*编辑时间  :
*备注      : 初版
***********************************************************************/
void EXTI4_15_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_LINE12) != RESET)
    {
        /*执行动作*/
        S_out_Flag = true;
        /*TODO*/
        EXTI_ClrITPendBit(EXTI_LINE12);
    }
}

/**ASW获取状态*************************************************************/
bool PortIf_Get_S_out_state(void)
{
    return S_out_Flag;
}

/**ASW清除漏电状态*********************************************************/
void PortIf_Clear_S_out_state(void)
{
    S_out_Flag = false;
}
