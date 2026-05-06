#ifndef __WDGIF_H__
#define __WDGIF_H__

#include "n32g05x.h"
#include "Wdgif_Cfg.h"
#include "n32g05x_iwdg.h"
#include "n32g05x_rcc.h"

/*Other Header File Inclusion*/

/* 外部引用配置表 */

/* 函数声明 */
extern void WdgIf_Init(void);
extern void WdgIf_DeInit(void);
extern void WdgIf_ClrStatusFlg(void);
extern void WdgIf_Trigger(void);
extern void WdgIf_SetMode(uint8_t WdgMode);

#endif /* __WDGIF_H__ */
