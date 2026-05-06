#include "Dioif.h"

/**********************************************************************
*函数名称  : void DioIf_Init(void)
*参数      : void
*返回值    : void
*描述      : Dio接口初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DioIf_Init(void)
{
    uint8_t TempDioChan;
    for(TempDioChan = 0u; TempDioChan < DIOIF_PIN_MAX_NUM; TempDioChan++)
    {
        DioIf_WriteChannel(stDioIfCfg[TempDioChan].ucChannel, stDioIfCfg[TempDioChan].ucInitPinLevel);
    }
}

/**********************************************************************
*函数名称  : void DioIf_DeInit(void)
*参数      : void
*返回值    : void
*描述      : Dio接口反初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DioIf_DeInit(void)
{
    uint8_t TempDioChan;
    for(TempDioChan = 0u; TempDioChan < DIOIF_PIN_MAX_NUM; TempDioChan++)
    {
        DioIf_WriteChannel(stDioIfCfg[TempDioChan].ucChannel, stDioIfCfg[TempDioChan].ucDeInitPinLevel);
    }
}

/**********************************************************************
*函数名称  : void DioIf_WriteChannel(uint8 ChannelId, uint8 Level)
*参数      : 频道索引: uint8 ChannelId, 层级: uint8 Level
*返回值    : void
*描述      : 频道写
*编辑时间  :
*备注      : 初版
***********************************************************************/
void DioIf_WriteChannel(uint16_t ChannelId, uint8_t Level)
{
    uint8_t TempPort = ChannelId>>4;
    switch (TempPort)
    {
        case (uint8_t)0x00:
            DioIfCfg_WritePin(GPIOA, ChannelId&(uint16_t)0xf, Level);
            break;
        case (uint8_t)0x01:
            DioIfCfg_WritePin(GPIOB, ChannelId&(uint16_t)0xf, Level);
            break;
        case (uint8_t)0x02:
            DioIfCfg_WritePin(GPIOC, ChannelId&(uint16_t)0xf, Level);
            break;
        case (uint8_t)0x03:
            DioIfCfg_WritePin(GPIOD, ChannelId&(uint16_t)0xf, Level);
            break;
        default:
            break;
    }
}

/**********************************************************************
*函数名称  : uint8 DioIf_ReadChannel(uint8 ChannelId)
*参数      : 频道索引: uint8 ChannelId
*返回值    : uint8 TempResult
*描述      : 从频道读
*编辑时间  :
*备注      : 初版
***********************************************************************/
uint8_t DioIf_ReadChannel(uint16_t ChannelId)
{
    uint8_t TempChannelId = ChannelId;
    uint8_t TempPort = TempChannelId>>4u;

    uint8_t TempResult = 0u;

    switch (TempPort)
    {
        case (uint8_t)0x00:
            TempResult = DioIfCfg_ReadPin(GPIOA, ChannelId&(uint16_t)0xf);
            break;
        case (uint8_t)0x01:
            TempResult = DioIfCfg_ReadPin(GPIOB, ChannelId&(uint16_t)0xf);
            break;
        case (uint8_t)0x02:
            TempResult = DioIfCfg_ReadPin(GPIOC, ChannelId&(uint16_t)0xf);
            break;
        case (uint8_t)0x03:
            TempResult = DioIfCfg_ReadPin(GPIOD, ChannelId&(uint16_t)0xf);
            break;
        default:
            break;
    }
    return (TempResult);
}
