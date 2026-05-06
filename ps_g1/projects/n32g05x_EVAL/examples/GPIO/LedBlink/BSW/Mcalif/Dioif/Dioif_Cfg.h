#ifndef __DIOIF_CFG_H__
#define __DIOIF_CFG_H__

#include "n32g05x.h"
#include "n32g05x_gpio.h"

/*Other Header File Inclusion*/
#define DIOIF_PIN_LOW        0u
#define DIOIF_PIN_HIGH       1u

#define DIOIF_PIN_MAX_NUM    5u

#define HW_DIO_PORTA_NUM     0u
#define HW_DIO_PORTB_NUM     16u
#define HW_DIO_PORTC_NUM     32u
#define HW_DIO_PORTD_NUM     48u

/*Output*/
#define HW_DIO_CH_OUT_SHRT       (HW_DIO_PORTD_NUM + 8u)  /* PD8  短路检测输出控制 */
#define HW_DIO_CH_CP_CONT        (HW_DIO_PORTB_NUM + 13u) /* PB13 CP控制 */
#define HW_DIO_CH_TEST_IN        (HW_DIO_PORTD_NUM + 13u) /* PD13 漏电模块控制 */
#define HW_DIO_CH_RELAY_ON       (HW_DIO_PORTD_NUM + 5u)  /* PD5  RELAY ON*/
#define HW_DIO_CH_RELAY_OFF      (HW_DIO_PORTD_NUM + 4u)  /* PD4  RELAY OFF*/

/*Input*/
#define HW_DIO_Rlychk_check      (HW_DIO_PORTA_NUM + 2u)  /* PA2  短路检测输入引脚 */
#define HW_DIO_CH_AC_DT          (HW_DIO_PORTB_NUM + 8u)  /* PB8  充放电检测IO*/
#define HW_DIO_CH_S_OUT          (HW_DIO_PORTD_NUM + 12u) /* PD12 漏电模块检测*/

typedef struct
{
    uint8_t ucChannel;            /* 通道 */
    uint8_t ucInitPinLevel;       /* 初始化电平 */
    uint8_t ucDeInitPinLevel;     /* 默认电平 */
}DioIfCfg_t;

/* 外部引用配置表 */
extern const DioIfCfg_t stDioIfCfg[DIOIF_PIN_MAX_NUM];

/* 函数声明 */
extern void DioIfCfg_WritePin(GPIO_Module* GPIO, uint16_t pin, uint8_t value);
extern uint8_t DioIfCfg_ReadPin(GPIO_Module* GPIO, uint16_t pin);

#endif /* __DIOIF_CFG_H__ */
