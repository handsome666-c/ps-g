#include "Dioif_Cfg.h"

/* DIO配置表 - 包含所有需要初始化的配置 */
const DioIfCfg_t stDioIfCfg[DIOIF_PIN_MAX_NUM] = 
{
    /* {uint8_t ucChannel, uint8_t ucInitPinLevel, uint8_t ucDeInitPinLevel} */
    {HW_DIO_CH_OUT_SHRT,    DIOIF_PIN_LOW,   DIOIF_PIN_LOW},
    {HW_DIO_CH_CP_CONT,     DIOIF_PIN_LOW,   DIOIF_PIN_LOW},
    {HW_DIO_CH_TEST_IN,     DIOIF_PIN_HIGH,  DIOIF_PIN_HIGH},
    {HW_DIO_CH_RELAY_ON,    DIOIF_PIN_LOW,   DIOIF_PIN_LOW},
    {HW_DIO_CH_RELAY_OFF,   DIOIF_PIN_HIGH,  DIOIF_PIN_HIGH},
};

/**********************************************************************
*函数名称  : void DioIfCfg_WritePin(GPIO_Module* GPIO, uint16_t pin, uint8_t value)
*参数      : port: base, pin: pin level:value
*返回值    : void
*描述      : 通道写
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DioIfCfg_WritePin(GPIO_Module* GPIO, uint16_t pin, uint8_t value)
{
    uint16_t pinMask = 0;
    pinMask = 1 << pin; /* 引脚掩码 */
    if(value == DIOIF_PIN_HIGH)
    {
        GPIO_SetBits(GPIO, pinMask); /* set pin */
    }
    else
    {
        GPIO_ResetBits(GPIO, pinMask); /* Reset pin */
    }
}

/**********************************************************************
*函数名称  : uint8_t DioIfCfg_ReadPin(GPIO_Module* GPIO, uint16_t pin)
*参数      : port: base, pin: pin
*返回值    : GPIO_ReadInputDataBit(GPIO, pinMask);
*描述      : 通道读
*编辑时间  :
*备注      : 初版
***********************************************************************/
uint8_t DioIfCfg_ReadPin(GPIO_Module* GPIO, uint16_t pin)
{
    uint16_t pinMask = 0;
    pinMask = 1 << pin; /* 引脚掩码 */
    return GPIO_ReadInputDataBit(GPIO, pinMask);
}

/* end */
