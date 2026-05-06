#ifndef __DIOIF_H__
#define __DIOIF_H__

#include "n32g05x.h"
#include "Dioif_Cfg.h"

/* 外部引用配置表 */

/* 函数声明 */
extern void DioIf_Init(void);
extern void DioIf_DeInit(void);
extern uint8_t DioIf_Rlychk_check(void);
extern void DioIf_WriteChannel(uint16_t ChannelId, uint8_t Level);
extern uint8_t DioIf_ReadChannel(uint16_t ChannelId);

#endif /* __DIOIF_CFG_H__ */
