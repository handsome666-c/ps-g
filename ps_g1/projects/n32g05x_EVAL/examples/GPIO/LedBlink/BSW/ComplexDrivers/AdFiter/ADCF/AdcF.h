#ifndef _ADCF_H_
#define _ADCF_H_

#include "AdcF_Cfg.h"

/* 函数声明 */
//#if (ADCFCFG_CLASS_BUDDLE_MIDDLE == ADCFCFG_ENABLE)
//extern void ADCF_BubbleSortMiddleConversion(uint8_t ucChannel, uint16_t usInsertValue);
//#endif
//#if (ADCFCFG_CLASS_BUDDLE_AVERAGE == ADCFCFG_ENABLE)
//extern void ADCF_BubbleSortAverageConversion(uint8_t ucChannel, uint16_t usInsertValue);
//#endif
//#if ( ADCFCFG_CLASS_SIMPLE_AVERAGE == ADCFCFG_ENABLE)
//void ADCF_SimpleAverageConversion(uint8_t ucChannel, uint16_t usInsertValue);
//#endif
//#if ( ADCFCFG_CLASS_FAST_AVERAGE == ADCFCFG_ENABLE)
//void ADCF_FastAverageConversion(uint8_t ucChannel, uint16_t usInsertValue);
//#endif

//extern uint16_t ADCF_GetValidValue(uint8_t ucChannel);
//extern uint16_t ADCF_GetCurrentValue(uint8_t ucChannel);

//extern void ADCF_InitMemory(void);
//
//extern uint16_t LIB_CalculateAverage(const uint16_t *pData, uint8_t length);
//extern void LIB_BubbleSort(uint16_t *pData, uint8_t length);

//extern uint32_t findMax(uint32_t arr[], uint8_t len);
extern void sortDesc(uint16_t arr[], int n);

extern uint16_t ADCF_CalculateIntAverage_Conversion(uint16_t* data, uint8_t length);

extern uint8_t threadHold_check_u32(uint32_t q1, uint32_t q2, uint32_t threadhold);

extern uint32_t hard_root(uint32_t ori);

#endif /* _ADCF_H_ */
