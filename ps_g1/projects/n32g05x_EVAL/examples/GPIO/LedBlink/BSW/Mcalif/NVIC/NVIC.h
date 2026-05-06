#ifndef __NVIC_H__
#define __NVIC_H__

#include "n32g05x.h"
#include "n32g05x_rcc.h"
#include "misc.h"

/*Other Header File Inclusion*/

/* 外部引用配置表 */

/* 函数声明 */
extern void NVIC_Configuration(IRQn_Type IRQn, uint8_t PRIORITY, FunctionalState cmd);

#endif /* __NVIC_H__ */
