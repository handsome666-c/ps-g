#ifndef __ECUM_H__
#define __ECUM_H__

#include "n32g05x.h"
#include "n32g05x_gpio.h"
#include "PortIf.h"
#include "DioIf.h"
#include "AdcIf.h"
#include "PwmIf.h"
#include "WdgM.h"
#include "Flashif.h"
#include "Timrif.h"
#include "AdcM.h"
#include "Uartif.h"
#include "RTE_ASW.h"

extern void EcuM_InitOne(void);
extern void EcuM_InitTwo(void);
extern void EcuM_InitThree(void);

#endif /* __ECUM_H__ */
