#include "Portif_Cfg.h"

/* Port配置表 - 包含所有需要初始化的引脚配置 */
const Port_settings_config_t g_PortInitConfigTable[PORT_CONFIGURED_NUM] =
{
    /* PD11 - pwm_LED1_G输出, 推挽输出, 无上拉下拉 */
    {
        .Port = LED1_G_PORT,
        .Pin = LED1_G_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM4
    },
    /* PC7 - pwm_LED2_B输出, 推挽输出, 无上拉下拉 */
    {
        .Port = LED2_B_PORT,
        .Pin = LED2_B_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM3
    },
    /* PC6 - pwm_LED3_R输出, 推挽输出, 无上拉下拉 */
    {
        .Port = LED3_R_PORT,
        .Pin = LED3_R_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM3
    },
    /* PD5 - IO_OUTPUT Relay_ON控制, 推挽输出, 无上拉下拉 */
    {
        .Port = Relay_ON_PORT,
        .Pin = Relay_ON_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PD4 - IO_OUTPUT Relay_OFF控制, 推挽输出, 无上拉下拉 */
    {
        .Port = Relay_OFF_PORT,
        .Pin = Relay_OFF_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PA11 - IO_OUTPUT Relay1控制, 推挽输出, 无上拉下拉 */
    {
        .Port = Relay1_PORT,
        .Pin = Relay1_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF1_TIM2
    },
    /* PD8 - IO_OUTPUT OUT_SHRT输出, 推挽输出, 无上拉下拉 */
    {
        .Port = OUT_SHRT_PORT,
        .Pin = OUT_SHRT_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PB13 - IO_OUTPUT CP_CONT输出, 推挽输出, 无上拉下拉 */
    {
        .Port = CP_CONT_PORT,
        .Pin = CP_CONT_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PD13 - IO_OUTPUT Test_in输出, 推挽输出, 无上拉下拉 */
    {
        .Port = Test_in_PORT,
        .Pin = Test_in_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PD12 - IO_INPUT S_out输入, 无上拉下拉 */
    {
        .Port = S_out_PORT,
        .Pin = S_out_PIN,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULL_UP,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PB8 - IO_INPUT AC_DT输入, 无上拉下拉 */
    {
        .Port = AC_DT_PORT,
        .Pin = AC_DT_PIN,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULL_DOWN,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PB4 - PWM_MCU_CP, GPIO_MODE_AF_PP, 无上拉下拉, GPIO_AF2_TIM3 */
    {
        .Port = PWM_MCU_CP_PORT,
        .Pin = PWM_MCU_CP_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM5
    },
    /* PA1 - ADC_NTC1, Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_NTC1_PORT,
        .Pin = ADC_NTC1_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PA0 - ADC_PGD接地检测, Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_PGD_PORT,
        .Pin = ADC_PGD_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PB0 - ADC_V_LINE_MCU检测(L\N Voltage Measure), Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_V_LINE_MCU_PORT,
        .Pin = ADC_V_LINE_MCU_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PA4-PA6 - ADC_EV_DIODE_CHK_MCU(车端二极管检测), Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_EV_DIODE_CHK_MCU_PORT,
        .Pin = ADC_EV_DIODE_CHK_MCU_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PA3 - ADC_CP_FB_AMP_MCU(CP幅值采样), Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_CP_FB_AMP_MCU_PORT,
        .Pin = ADC_CP_FB_AMP_MCU_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PB3 - ADC_I_CS(电流采样), Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_I_CS_PORT,
        .Pin = ADC_I_CS_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    /* PB12 - ADC_RLY_CHK_MCU(继电器粘连), Analog Mode, 无上拉下拉 */
    {
        .Port = ADC_RLY_CHK_MCU_PORT,
        .Pin = ADC_RLY_CHK_MCU_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    }
};

const Port_settings_config_t g_Port_CP_ConfigTable[PORT_CONFIG_MCUCP_NUM] =
{
    {
        .Port = PWM_MCU_CP_PORT,
        .Pin = PWM_MCU_CP_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },/*MCU_CP GPIO*/
    {
        .Port = PWM_MCU_CP_PORT,
        .Pin = PWM_MCU_CP_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM5
    }/*MCU_CP TMR5_CH3*/
};

const Port_settings_config_t g_Port_Rly_ConfigTable[PORT_CONFIG_Rly_NUM] =
{
    {
        .Port = Relay1_PORT,
        .Pin = Relay1_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },
    {
        .Port = Relay1_PORT,
        .Pin = Relay1_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM2
    }
};

const Port_settings_config_t g_Port_LED_ConfigTable[PORT_CONFIG_LED_NUM] =
{
    {
        .Port = LED1_G_PORT,
        .Pin = LED1_G_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },/*MCU_LEDRGB GPIO*/
    {
        .Port = LED2_B_PORT,
        .Pin = LED2_B_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },/*MCU_LEDRGB GPIO*/
    {
        .Port = LED3_R_PORT,
        .Pin = LED3_R_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_FAST,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_NO_AF
    },/*MCU_LEDRGB GPIO*/
    {
        .Port = LED1_G_PORT,
        .Pin = LED1_G_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM4
    },/*MCU LEDRGB TMR4 CH1*/
    {
        .Port = LED2_B_PORT,
        .Pin = LED2_B_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM3
    },/*MCU LEDRGB TMR4 CH1*/
    {
        .Port = LED3_R_PORT,
        .Pin = LED3_R_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NO_PULL,
        .Slew_rate = GPIO_SLEW_RATE_SLOW,
        .Current = GPIO_HIGH_DREIVE,
        .Alternate = GPIO_AF2_TIM3
    }/*MCU LEDRGB TMR4 CH1*/
};

/**********************************************************************
*函数名称  : void PortIfCfg_PinDrv_Init(const Port_settings_config_t* ConfigPtr)
*参数      : ConfigPtr 配置表结构体
*返回值    : void
*描述      : 引脚初始化
*编辑时间  :
*初版      :
*备注      :
***********************************************************************/
void PortIfCfg_PinDrv_Init(const Port_settings_config_t ConfigPtr[])
{
    GPIO_InitType GPIO_InitStructure;

    for (uint8_t i = 0; i < PORT_CONFIGURED_NUM; i++)
    {
        if(ConfigPtr[i].Port == GPIOA) {
            RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
        } else if (ConfigPtr[i].Port == GPIOB) {
            RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
        } else if (ConfigPtr[i].Port == GPIOC) {
            RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
        } else if (ConfigPtr[i].Port == GPIOD) {
            RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOD, ENABLE);
        } else {
            return;
        }

        GPIO_InitStruct(&GPIO_InitStructure);
        GPIO_InitStructure.Pin = ConfigPtr[i].Pin;
        GPIO_InitStructure.GPIO_Mode = ConfigPtr[i].Mode;
        GPIO_InitStructure.GPIO_Pull = ConfigPtr[i].Pull;
        GPIO_InitStructure.GPIO_Slew_Rate = ConfigPtr[i].Slew_rate;
        GPIO_InitStructure.GPIO_Current = ConfigPtr[i].Current;
        GPIO_InitStructure.GPIO_Alternate = ConfigPtr[i].Alternate;
        GPIO_InitPeripheral(ConfigPtr[i].Port, &GPIO_InitStructure);
    }
}
