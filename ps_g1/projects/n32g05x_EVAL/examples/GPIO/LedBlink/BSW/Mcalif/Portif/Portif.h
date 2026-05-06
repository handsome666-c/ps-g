#ifndef __PORTIF_H__
#define __PORTIF_H__

#include "n32g05x.h"
#include "Portif_Cfg.h"
#include "n32g05x_rcc.h"
#include "n32g05x_gpio.h"
#include "n32g05x_exti.h"
#include "NVIC.h"
#include <stdbool.h>

/*Other Header File Inclusion*/
extern bool S_out_Flag;

/* 函数声明 */
extern void PortIf_Init(void);
extern void PortIf_SetPinMode(const Port_settings_config_t ConfigPtr);
extern void PortIf_PortEXTI12_Enable(void);
extern void PortIf_PortEXTI_Enable(IRQn_Type IRQn, uint8_t PRIORITY);
extern void PortIf_PortEXTI_Disable(IRQn_Type IRQn);
extern bool PortIf_Get_S_out_state(void);
extern void PortIf_Clear_S_out_state(void);

#endif /* __PORTIF_H__ */
