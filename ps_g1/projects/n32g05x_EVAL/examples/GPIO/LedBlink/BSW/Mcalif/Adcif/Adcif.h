#ifndef __ADCIF_H__
#define __ADCIF_H__

#include "n32g05x.h"
#include "n32g05x_rcc.h"
#include "n32g05x_adc.h"
#include "n32g05x_dma.h"
#include "Adcif_Cfg.h"
#include "NVIC.h"
#include "misc.h"
#include "AdcM_Cfg.h"
#include "AdcM.h"
#include "rtthread.h"
#include <math.h>

/*Other Header File Inclusion*/
#define CH_NUM    5       /* 每轮实际采样个数 */
#define ROUND     4       /* 3 轮循环 */

/* 外部引用配置表 */
//extern uint16_t AdcIf_ConvResult[ROUND][CH_NUM];
extern uint32_t AdcIf_ConvResult[5];

/* 函数声明 */
extern void AdcIf_Init(void);
extern void AdcIf_StartConv(void);
extern void AdcIf_DeInit(void);
extern bool AdcIf_GetConvCompleteFlag(void);
extern void AdcIf_SetConvCompleteFlag(void);
extern void AdcIf_GetConvResult(uint32_t* Readbuf);

#endif /* __ADCIF_H__ */
