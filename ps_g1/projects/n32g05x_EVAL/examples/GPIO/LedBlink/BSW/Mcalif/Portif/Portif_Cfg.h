#ifndef __PORTIF_CFG_H__
#define __PORTIF_CFG_H__

#include "n32g05x.h"
#include "n32g05x_rcc.h"
#include "n32g05x_gpio.h"

/*Other Header File Inclusion*/
#define PortIf_DISABLE    0u
#define PortIf_ENABLE     1u

/*PORT*/
#define OUT_SHRT_PORT      GPIOD   /*OUT_SHRT*/
#define OUT_SHRT_PIN       GPIO_PIN_8

#define CP_CONT_PORT       GPIOB   /*CP_CONT*/
#define CP_CONT_PIN        GPIO_PIN_13

#define AC_DT_PORT         GPIOB   /*AC_DT*/
#define AC_DT_PIN          GPIO_PIN_8

#define Test_in_PORT       GPIOD   /*Test_in 漏电保护*/
#define Test_in_PIN        GPIO_PIN_13

#define S_out_PORT         GPIOD   /*S_out 漏电保护*/
#define S_out_PIN          GPIO_PIN_12

//#define Rlychk_check_PORT  GPIOA   /*自检完成输入MCU*/
//#define Rlychk_check_PIN   GPIO_PIN_2

#define Relay_ON_PORT      GPIOD   /*Relay*/
#define Relay_ON_PIN       GPIO_PIN_5

#define Relay_OFF_PORT     GPIOD   /*Relay*/
#define Relay_OFF_PIN      GPIO_PIN_4

/*PWM*/
#define LED1_G_PORT        GPIOD   /*Led1*/
#define LED1_G_PIN         GPIO_PIN_11

#define LED2_B_PORT        GPIOC   /*Led2*/
#define LED2_B_PIN         GPIO_PIN_7

#define LED3_R_PORT        GPIOC   /*Led3*/
#define LED3_R_PIN         GPIO_PIN_6

#define Relay1_PORT        GPIOA   /*Relay1*/
#define Relay1_PIN         GPIO_PIN_11

#define PWM_MCU_CP_PORT    GPIOB
#define PWM_MCU_CP_PIN     GPIO_PIN_4

/*ADC*/
#define ADC_NTC1_PORT      GPIOA
#define ADC_NTC1_PIN       GPIO_PIN_1

#define ADC_PGD_PORT       GPIOC
#define ADC_PGD_PIN        GPIO_PIN_0

#define ADC_V_LINE_MCU_PORT  GPIOB  /*L\N Voltage Measure*/
#define ADC_V_LINE_MCU_PIN   GPIO_PIN_0

#define ADC_EV_DIODE_CHK_MCU_PORT  GPIOA /*车端二极管检测*/
#define ADC_EV_DIODE_CHK_MCU_PIN   GPIO_PIN_4

#define ADC_CP_FB_AMP_MCU_PORT  GPIOA /*CP幅值采样*/
#define ADC_CP_FB_AMP_MCU_PIN   GPIO_PIN_3

#define ADC_I_CS_PORT        GPIOB /*电流采样*/
#define ADC_I_CS_PIN         GPIO_PIN_3

#define ADC_RLY_CHK_MCU_PORT  GPIOB /*继电器粘连采样*/
#define ADC_RLY_CHK_MCU_PIN   GPIO_PIN_12

#define PORT_CONFIGURED_NUM    19U
#define PORT_CONFIG_MCUCP_NUM  2U
#define PORT_CONFIG_Rly_NUM    2U
#define PORT_CONFIG_LED_NUM    6U

/* 引脚配置结构体 */
typedef struct {
    GPIO_Module* Port;         /** GPIO **/
    uint32_t Pin;              /** GPIO_pins_define **/
    uint32_t Mode;             /** GPIO_mode_define **/
    uint32_t Pull;             /** GPIO Pull-Up or Pull-Down Activation **/
    uint32_t Slew_rate;        /** slew rate config **/
    uint32_t Current;         /** driver strength config **/
    uint32_t Alternate;
} Port_settings_config_t;

/* 外部引用配置表 */
extern const Port_settings_config_t g_PortInitConfigTable[PORT_CONFIGURED_NUM];
extern const Port_settings_config_t g_Port_CP_ConfigTable[PORT_CONFIG_MCUCP_NUM];
extern const Port_settings_config_t g_Port_Rly_ConfigTable[PORT_CONFIG_Rly_NUM];
extern const Port_settings_config_t g_Port_LED_ConfigTable[PORT_CONFIG_LED_NUM];

/* 函数声明 */
extern void PortIfCfg_PinDrv_Init(const Port_settings_config_t* ConfigPtr);

#endif /* __PORTIF_CFG_H__ */
