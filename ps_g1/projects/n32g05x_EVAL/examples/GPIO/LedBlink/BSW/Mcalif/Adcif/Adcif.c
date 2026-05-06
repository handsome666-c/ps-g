#include "Adcif.h"

// ADC_Channel_08_PB0    Voltage
// ADC_Channel_14_PB3    I_CS
// ADC_Channel_03_PA3    CP幅值、占空比检测
// ADC_Channel_15_TS     mcutemp
// ADC_Channel_00_PA0    接地检测通道
// ADC_Channel_04_PA4    车端二极管检测通道
// ADC_Channel_13_PB12   继电器粘连
// ADC_Channel_01_PA1    NTC1
// ADC_Channel_02_PA2    短路检测
#define ADC_CHANNELS         9      // 总共9路通道
const uint8_t SeqTbl[ADC_CHANNELS] = {ADC_Channel_08_PB0, ADC_Channel_14_PB3, ADC_Channel_03_PA3, ADC_Channel_15_TS,
                                      ADC_Channel_00_PA0, ADC_Channel_04_PA4, ADC_Channel_13_PB12, ADC_Channel_01_PA1, ADC_Channel_02_PA2};  /* 轮 0 */
uint32_t AdcIf_ConvResult[5] = {0};
static uint16_t current_channel_group_count = 0;
static uint8_t current_channel_group = 4;

/**********************************************************************
*函数名称  : void RCC_ADC_Configuration(void)
*参数      : void
*返回值    : void
*描述      : Clock
*编辑时间  :
*备注      : 初版
***********************************************************************/
static void RCC_ADC_Configuration(void)
{
    /* RCC_ADCHCLK_DIV1*/
    RCC_ConfigHclk(RCC_SYSCLK_DIV1);
//  /* Enable GPIOA and TIM1 clocks */
//  RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_TIM1,ENABLE);
    /* Enable ADC and DMA clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC | RCC_AHB_PERIPH_DMA,ENABLE);
    ADC_ClockModeConfig(ADC_CKMOD_AHB,RCC_ADCHCLK_DIV16);
    /* enable ADC 1M clock */
//  RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI,RCC_ADC1MCLK_DIV8);
}

/**********************************************************************
*函数名称  : static void DMA_Init(void)
*参数      : void
*返回值    : void
*描述      : DMA初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
static void DMA_Config(void)
{
    DMA_InitType DMA_InitStructure;
    DMA_DeInit(DMA_CH1);
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr      = (uint32_t)&ADC->DAT0;  /*采样周期*/
    DMA_InitStructure.MemAddr         = (uint32_t)AdcIf_ConvResult;/*指定了 DMA 通道 x 的内存地址*/
    DMA_InitStructure.Direction       = DMA_DIR_PERIPH_SRC;/*指明该外围设备是作为源设备还是目标设备使用*/
    DMA_InitStructure.BufSize         = 5;
    DMA_InitStructure.PeriphInc       = DMA_PERIPH_INC_ENABLE;
    DMA_InitStructure.DMA_MemoryInc   = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize  = DMA_PERIPH_DATA_SIZE_WORD;
    DMA_InitStructure.MemDataSize     = DMA_MemoryDataSize_Word;
    DMA_InitStructure.CircularMode    = DMA_MODE_CIRCULAR;
    DMA_InitStructure.Priority        = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem          = DMA_M2M_DISABLE;
    DMA_Init(DMA_CH1, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_ADC, DMA, DMA_CH1, ENABLE);
    DMA_ConfigInt(DMA_CH1, DMA_INT_TXC, ENABLE);
//  DMA_ConfigInt(DMA_CH1, DMA_INT_HTX, ENABLE);
    DMA_EnableChannel(DMA_CH1, ENABLE);
}

/**********************************************************************
*函数名称  : void AdcIf_Init(void)
*参数      : void
*返回值    : void
*描述      : 模拟数字转换接口初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void AdcIf_Init(void)
{
    ADC_InitType ADC_InitStructure;

    RCC_ADC_Configuration();

    NVIC_Configuration(DMA_CH1_2_IRQn, NVIC_PRIORITY_0, ENABLE);

    /* DMA configuration ------------------------------------------*/
    DMA_Config();

    ADC_InitStruct(&ADC_InitStructure);
    /* ADC configuration ------------------------------------------*/
    ADC_InitStructure.MultiChEn        = ENABLE;
    ADC_InitStructure.ContinueConvEn    = ENABLE;
    ADC_InitStructure.ExtTrigSelect     = ADC_EXT_TRIGCONV_REGULAR_SWSTRRCH;
    ADC_InitStructure.DatAlign          = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber         = ADC_REGULAR_LEN_5;
    ADC_Init(&ADC_InitStructure);

    /* ADC regular sequence register configures channelx */
    ADC_ConfigRegularSequenceMulti(SeqTbl[0], ADC_DATA_OFFSET0+0);
    ADC_ConfigRegularSequenceMulti(SeqTbl[1], ADC_DATA_OFFSET0+1);
    ADC_ConfigRegularSequenceMulti(SeqTbl[2], ADC_DATA_OFFSET0+2);
    ADC_ConfigRegularSequenceMulti(SeqTbl[3], ADC_DATA_OFFSET0+3);
    ADC_ConfigRegularSequenceMulti(SeqTbl[4], ADC_DATA_OFFSET0+4);

    /* Configure channel sample time
    64M/16=4M(周期0.25μs)
    单通道转换时间:
    采样时间+转换时间(12周期)
    选择采样时间为8个周期: 8+12=20个周期
    20×0.25=5μs
    5个通道总时间: 5×5=25μs
    1/25 = 0.04(40khz)
    40/5通道 = 8khz
    -----------------------------*/
    /* Enable ADC DMA */
    ADC_EnableDMA(ENABLE);

    /* Enable ADC TempSensor */
    ADC_EnableTempSensor(ENABLE);

    ADC_ConfigChannelSampleTime(ADC_SAMP_TIME_88CYCLES);

    /* Enable ADC */
    ADC_Enable(ENABLE);

    /* Check ADC Ready */
    while(ADC_GetFlagStatusNew(ADC_FLAG_RDY) == RESET)
        ;
    ADC_EnableSoftwareStartConv(ENABLE);
}

/**********************************************************************
*函数名称  : void AdcIf_DeInit(void)
*参数      : void
*返回值    : void
*描述      : 模拟数字转换接口反初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void AdcIf_DeInit(void)
{
    /* Disable ADC DMA */
    ADC_EnableDMA(DISABLE);
    /* Disable ADC */
    ADC_Enable(DISABLE);
}

/**********************************************************************
*函数名称  : void ADC_Update_Sequence(void)
*参数      : void
*返回值    : void
*描述      : 更新通道序列: 替换最后一个通道
*编辑时间  :
*备注      : 初版
***********************************************************************/
void ADC_Update_Sequence(void)
{
    current_channel_group_count += 1;
    if(current_channel_group_count >= 199)
    {
        current_channel_group++;
        if (current_channel_group > 4+4) {
            current_channel_group = 4;
        }
        ADC_ConfigRegularSequenceMulti(SeqTbl[current_channel_group], ADC_DATA_OFFSET0 + 4);
    }
}

/**********************************************************************
*函数名称  : void DMA_CH1_2_IRQHandler(void)
*参数      : void
*返回值    : void
*描述      : DMA_CH1_2_IRQHandler
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DMA_CH1_2_IRQHandler(void)
{
//  if(DMA_GetIntStatus(DMA_FLAG_HT1, DMA) != RESET)
//  {
//      /*清除中断标志位（需配合DMA_ClrIntPendingBit使用）*/
//      DMA_ClrIntPendingBit(DMA_FLAG_HT1, DMA);
//  }

    // 检查 DMA 通道1是否传输完成
    if(DMA_GetIntStatus(DMA_FLAG_TC1, DMA) != RESET)
    {
        /*清除中断标志位（需配合DMA_ClrIntPendingBit使用）*/
        DMA_ClrIntPendingBit(DMA_FLAG_TC1, DMA);

        AdcM_get_Voltage_update(&ADCM_t,AdcIf_ConvResult[0] & 0xffff);
        AdcM_get_Current_update(&ADCM_t,AdcIf_ConvResult[1] & 0xffff);
        AdcM_get_CP_update(&ADCM_t, AdcIf_ConvResult[2] & 0xffff);
        AdcM_get_MCUTemp_update(&ADCM_t,AdcIf_ConvResult[3] & 0xffff);

        switch(current_channel_group)
        {
            case 4:
                AdcM_get_Ground_update(&ADCM_t, AdcIf_ConvResult[4] & 0xffff);
                break;
            case 5:
                AdcM_get_EvdiodeCHK_update(&ADCM_t, AdcIf_ConvResult[4] & 0xffff);
                break;
            case 6:
                AdcM_get_RelayCHK_update(&ADCM_t, AdcIf_ConvResult[4] & 0xffff);
                break;
            case 7:
                AdcM_get_NTC1Temp_update(&ADCM_t, AdcIf_ConvResult[4] & 0xffff);
                break;
            case 8:
                AdcM_get_ShortCircuit_update(&ADCM_t, AdcIf_ConvResult[4] & 0xffff);
                break;
        }
        ADC_Update_Sequence();  //ADC通道更新
    }
}
