#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <stdio.h>
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "n32g05x.h"
#include "n32g05x_gpio.h"
#include "n32g05x_rcc.h"
#include "RTE_BSW.h"
#include "Flashif.h"

typedef struct
{
    uint32_t readBuffer[2];
} factory_Manager_t;

extern factory_Manager_t factory_Manager;
extern bool get_factory_mode_status(factory_Manager_t *factory);

#endif
