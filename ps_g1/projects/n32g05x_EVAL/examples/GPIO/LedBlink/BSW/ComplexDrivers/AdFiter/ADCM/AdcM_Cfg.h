#ifndef _ADCM_CFG_H_
#define _ADCM_CFG_H_

#include "n32g05x.h"
#include "AdcF.h"

typedef enum
{
    ADCMCFG_Voltage_CH,/* 使能V_LINE_MCU检测*/
    ADCMCFG_Currnet_CH,
    ADCMCFG_CP_CH,/* 使能CP_FB_AMP检测*/
    ADCMCFG_NTC1_CH, /* 使能TEMP_CP_NTC1检测*/
    ADCMCFG_NTC1Temp_CH, /* 使能TEMP_CP_NTC1检测*/
    ADCMCFG_Ground_CH,  /* 使能PGD检测*/
    ADCMCFG_EvdiodeCHK_CH, /* 使能车端二极管检测*/
    ADCMCFG_RelayCHK_CH,
    ADCMCFG_MCUTemp_CH,
    ADCMCFG_Shortcircuit_CH,
    ADCMCFG_RMSVOLX1000,
    ADCMCFG_RMSCURX1000,
//  ADCMCFG_rmscurmaxX1000,
    ADCMCFG_CH_MAX_NUM
	
} AdcCfg_ChanEnumType;

#endif /* __ADCM_CFG_H__ */


