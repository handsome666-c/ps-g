#ifndef __TIMRIF_H__
#define __TIMRIF_H__

#include "n32g05x.h"
#include "n32g05x_rcc.h"
#include "n32g05x_tim.h"
//#include "Timrif_Cfg.h"
#include "NVIC.h"
#include "misc.h"
#include "WdgM.h"

/*Other Header File Inclusion*/
extern uint32_t TmrIf_systick_cnt;

/* 外部引用配置表 */

/* 函数声明 */
extern void TmrIf_Init(void);
extern void TmrIf_StartTimer(void);
extern void TmrIf_DeInit(void);
extern void TmrIf_DelayMs(uint32_t ms);

#endif /* __TIMRIF_H__ */
