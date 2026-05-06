#ifndef __RTE_BSW_H__
#define __RTE_BSW_H__

#include "PortIf.h"
#include "WdgM.h"
#include "DioIf.h"
#include "PwmIf.h"
#include "AdcM.h"
#include "rtthread.h"
#include "UartIf.h"
#include "Flashif.h"

/*Other Header File Inclusion*/

/* 函数声明 */

/*PortIf*/
//extern bool RTE_Call_PortIf_Rlychk_check_state(void);
extern bool RTE_Call_PortIf_Get_S_out_state(void);
extern void RTE_Call_PortIf_Clear_S_out_state(void);

/*DioIf*/
extern void RTE_Call_DioIf_WriteChannel(uint16_t ChannelId,uint8_t Level);
extern uint8_t RTE_Call_DioIf_ReadChannel(uint16_t ChannelId);

/*PwmIf*/
extern void RTE_Call_PwmIf_Set_Duty(TIM_Module* tmr, uint16_t OCy, uint16_t Duty);

/*AdcIf*/
extern int32_t RTE_Call_AdcM_Vlotage(AdcCfg_ChanEnumType Channel);

/*NVM*/
extern void RTE_Call_NVM_AppWriteBlock(uint16_t BlockID, const uint8_t WriteData[]);
extern uint8_t RTE_Call_NVM_AppReadBlock(uint16_t BlockID, uint8_t ReadData[]);

/// /*FlsIf*/
extern void RTE_Call_FlsIf_ErasePage(uint32_t pageAddr);
extern void RTE_Call_FlsIf_ProgramMultiple(uint32_t address, const uint32_t* pBuffer, uint32_t len);
extern void RTE_Call_FlsIf_ReadMultiple(uint32_t address, uint32_t* buf, uint32_t len);

#endif /* __RTE_BSW_H__ */
