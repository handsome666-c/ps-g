#include "RTE_BSW.h"

/*************************PortIf模块*************************/
bool RTE_Call_PortIf_Get_S_out_state(void)
{
    return PortIf_Get_S_out_state();
}

void RTE_Call_PortIf_Clear_S_out_state(void)
{
    PortIf_Clear_S_out_state();
}

/*************************WDGM模块*************************/
void RTE_Call_WdgM_Trigger(void)
{
    WdgM_Trigger();
}

/*************************PWMIF模块*************************/
void RTE_Call_PwmIf_Set_Duty(TIM_Module* tmr, uint16_t OCy,  uint16_t Duty)
{
    PwmIf_Set_Duty(tmr, OCy, Duty);
}

/*************************DioIf模块*************************/
void RTE_Call_DioIf_WriteChannel(uint16_t ChannelId, uint8_t Level)
{
    DioIf_WriteChannel(ChannelId, Level);
}

uint8_t RTE_Call_DioIf_ReadChannel(uint16_t ChannelId)
{
    return DioIf_ReadChannel(ChannelId);
}


/*************************AdcM模块*************************/
int32_t RTE_Call_AdcM_Vlotage(AdcCfg_ChanEnumType Channel)
{
    return AdcM_GetValidValue(Channel);
}

/*************************FlashIf模块*************************/
void RTE_Call_FlsIf_ErasePage(uint32_t pageAddr)
{
    FlsIf_ErasePage(pageAddr);
}

void RTE_Call_FlsIf_ProgramMultiple(uint32_t address, const uint32_t* pBuffer, uint32_t len)
{
    FlsIf_ProgramMultiple(address, pBuffer, len);
}

void RTE_Call_FlsIf_ReadMultiple(uint32_t address, uint32_t* buf, uint32_t len)
{
    FlsIf_ReadMultiple(address, buf, len);
}
