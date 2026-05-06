#include "Wdgif.h"

static uint8_t WdgIf_Mode = WDGIF_MODE_OFF;

__IO uint32_t LsiFreq = 32000;

/**********************************************************************
*函数名称  : void WdgIf_ClrStatusFlg(void)
*参数      : void
*返回值    : void
*描述      : 检查WDG
*编辑时间  :
*备注      : 初版
***********************************************************************/
void WdgIf_ClrStatusFlg(void)
{
    if(WdgIf_Mode == WDGIF_MODE_OFF)
    {

    }
    else if(WdgIf_Mode == WDGIF_MODE_IWDT)
    {
        /* Check if the system has resumed from IWDG reset */
        if (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_IWDGRSTF) != RESET)
        {
            /* Clear reset flags */
            RCC_ClearResetFlag();
        }
    }
    else if(WdgIf_Mode == WDGIF_MODE_WWDT)
    {
        /* Check if the system has resumed from WWDG reset */
        if (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_WWDGRSTF) != RESET)
        {
            /* Clear reset flags */
            RCC_ClearResetFlag();
        }
    }
    else
    {
        //do nothing
    }
}

/**********************************************************************
*函数名称  : void WdgIf_Init(void)
*参数      : void
*返回值    : void
*描述      : WDG初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void WdgIf_Init(void)
{
    if(WdgIf_Mode == WDGIF_MODE_OFF)
    {

    }
    else if(WdgIf_Mode == WDGIF_MODE_IWDT)
    {
        /* Enable PWR Clock */
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
        WdgIf_ClrStatusFlg();
        /* Enable write access to IWDG_PR and IWDG_RLR registers */
        IWDG_WriteConfig(IWDG_WRITE_ENABLE);
        /* IWDG counter clock: LSI/32 */
        IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);
        IWDG_CntReload(LsiFreq / 320);
        /* Reload IWDG counter */
        IWDG_ReloadKey();
        /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
        IWDG_Enable();
    }
    else if(WdgIf_Mode == WDGIF_MODE_WWDT)
    {
        WWDG_ClrEWINTF();
        /* Enable PWR Clock */
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
        WdgIf_ClrStatusFlg();
        /* Enable WWDG clock */
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_WWDG, ENABLE);
        /* WWDG clock counter = 1/((PCLK1(32MHz)/4096)/8) = (~1024 us) */
        WWDG_SetPrescalerDiv(WWDG_PRESCALER_DIV8);
        WWDG_SetWValue(80);
        /*Enable WWDG */
        WWDG_Enable(127);
    }
    else
    {
        //do nothing
    }
}

/**********************************************************************
*函数名称  : void WdgIf_Trigger(void)
*参数      : void
*返回值    : void
*描述      : WDG重载
*编辑时间  :
*备注      : 初版
***********************************************************************/
void WdgIf_Trigger(void)
{
    if(WdgIf_Mode == WDGIF_MODE_OFF)
    {

    }
    else if(WdgIf_Mode == WDGIF_MODE_IWDT)
    {
        /* Reload IWDG counter */
        IWDG_ReloadKey();
    }
    else if(WdgIf_Mode == WDGIF_MODE_WWDT)
    {
        /* Update WWDG counter */
        WWDG_SetCnt(127);
    }
    else
    {
        //do nothing
    }
}

/**********************************************************************
*函数名称  : void WdgIf_DeInit(void)
*参数      : void
*返回值    : void
*描述      : WDG反初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void WdgIf_DeInit(void)
{
    if(WdgIf_Mode == WDGIF_MODE_OFF)
    {

    }
    else if(WdgIf_Mode == WDGIF_MODE_IWDT)
    {

    }
    else if(WdgIf_Mode == WDGIF_MODE_WWDT)
    {
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_WWDG, ENABLE);
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_WWDG, DISABLE);
    }
    else
    {
        //do nothing
    }
}

/**********************************************************************
*函数名称  : void WdgIf_SetMode(uint8_t WdgMode)
*参数      : WdgMode
*返回值    : void
*描述      : WDG初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void WdgIf_SetMode(uint8_t WdgMode)
{
    WdgIf_Mode = WdgMode;
}
