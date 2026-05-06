#ifndef __RTE_ASW_H__
#define __RTE_ASW_H__

//#include "WdgM.h"
//#include "DioIf.h"
//#include "PwmIf.h"
//#include "AdcM.h"
/*Other Header File Inclusion*/

#include "charging.h"
#include "ChargeManager.h"
#include "Led.h"
#include "Log.h"
#include "Diag.h"

/* 函数声明 */

extern void RTE_Call_ASW_Init(void);
extern void RTE_Call_ASW_MainFunction_1ms(void);
extern void RTE_Call_ASW_MainFunction_5ms(void);
extern void RTE_Call_ASW_MainFunction_10ms(void);
extern void RTE_Call_ASW_MainFunction_1s(void);

#endif /* __RTE_ASW_H__ */
