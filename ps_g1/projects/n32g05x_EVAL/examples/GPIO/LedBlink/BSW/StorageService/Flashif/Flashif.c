#include "Flashif.h"

#define DFLASH_START_ADDR      ((uint32_t)0x1FFF1000)
#define DFLASH_SIZE            ((uint16_t)0x2000)

#define DFLASH_PAGE_SIZE       ((uint16_t)0x200)

static uint32_t readBufferUID[4];

/**********************************************************************
*函数名称 : void FlsIf_ErasePage(uint32_t pageAddr)
*参数      : uint32_t pageAddr
*返回值    : void
*描述      : 擦除对应page
*编辑时间  :
*备注      : 初版 (主存储区可以按页擦除或者整片擦除，DATA flash 区仅支持按页擦除)
***********************************************************************/
void FlsIf_ErasePage(uint32_t pageAddr)
{
    FLASH_Unlock();
    if (FLASH_EOP != FLASH_EraseOnePage(pageAddr))
    {
        while(1)
        {
            rt_kprintf("Flash EraseOnePage Error. Please Deal With This Error Promptly\r\n");
        }
    }
    FLASH_Lock();
}

/**********************************************************************
*函数名称 : void FlsIf_ProgramPage(uint32_t address, const uint32_t* pBuffer)
*参数      : uint32_t address, const uint32_t* pBuffer
*返回值    : void
*描述      : Flash按页写入
*编辑时间  :
*备注      : 初版
*注意      : 写入前必须先执行擦除，Flash特性：只能从1写0
***********************************************************************/
void FlsIf_ProgramPage(uint32_t address, const uint32_t* pBuffer)
{
    FLASH_Unlock();
    for (uint32_t i = 0; i < DFLASH_PAGE_SIZE; i += 8)
    {
        // 写入双字数据（两个32位字）
        if (FLASH_EOP != FLASH_ProgramdoubleWord(address + i, pBuffer[i/4], pBuffer[i/4 + 1]))
        {
            while(1)
            {
                rt_kprintf("Flash ProgramWord Error. Please Deal With This Error Promptly\r\n");
            }
        }
    }
    FLASH_Lock();
}

/**********************************************************************
*函数名称 : void FlsIf_ProgramMultiple(uint32_t address, const uint32_t* pBuffer, uint32_t len)
*参数      : uint32_t address, const uint32_t* pBuffer, uint32_t len
*返回值    : void
*描述      : FLASH按需求长度写入
*编辑时间  :
*备注      : 初版
*注意      : 写入前必须先执行擦除，Flash特性：只能从1写0
***********************************************************************/
void FlsIf_ProgramMultiple(uint32_t address, const uint32_t* pBuffer, uint32_t len)
{
    // 检查数据长度是否为8的整数倍（Flash_write函数要求）
    if (len % 8 != 0)
    {
        rt_kprintf("错误：数据长度必须是8的整数倍！\r\n");
        return;
    }
    FLASH_Unlock();
    for (uint32_t i = 0; i < len; i += 8)
    {
        // 写入双字数据（两个32位字）
        if (FLASH_EOP != FLASH_ProgramdoubleWord(address + i, pBuffer[i/4], pBuffer[i/4 + 1]))
        {
            while(1)
            {
                rt_kprintf("Flash ProgramWord Error. Please Deal With This Error Promptly\r\n");
            }
        }
    }
    FLASH_Lock();
}

/**********************************************************************
*函数名称 : void FlsIf_ReadPage(uint32_t address, uint32_t* buf)
*参数      : uint32_t address, uint32_t* buf
*返回值    : void
*描述      : FLASH读取Page
*编辑时间  :
*备注      : 初版
*注意      : 写入前必须先执行擦除，Flash特性：只能从1写0
***********************************************************************/
void FlsIf_ReadPage(uint32_t address, uint32_t* buf)
{
    for (uint32_t i = 0; i < DFLASH_PAGE_SIZE; i += 8)
    {
        buf[i] = *(__IO uint32_t*)(address + i);
        // rt_kprintf("Flash Read Page Test ok\r\n");
    }
}

/**********************************************************************
*函数名称 : void FlsIf_ReadMultiple(uint32_t address, uint32_t* buf, uint32_t len)
*参数      : uint32_t address, uint32_t* buf, uint32_t len
*返回值    : void
*描述      : FLASH按需求长度读取
*编辑时间  :
*备注      : 初版
*注意      : 写入前必须先执行擦除，Flash特性：只能从1写0
***********************************************************************/
void FlsIf_ReadMultiple(uint32_t address, uint32_t* buf, uint32_t len)
{
    // 检查数据长度是否为8的整数倍（Flash_write函数要求）
    if (len % 8 != 0)
    {
        rt_kprintf("错误：数据长度必须是8的整数倍！\r\n");
        return;
    }
    for (uint32_t i = 0; i < len; i += 4)
    {
        buf[i/4] = *(__IO uint32_t*)(address + i);
        // rt_kprintf("Flash Read Multiple Test ok\r\n");
    }
}

/**
 * @brief CRC32计算（用于校准数据完整性校验）
 * @param data 数据指针
 * @param length 数据长度（字节）
 * @return CRC32值
 */
uint32_t calibration_crc32(const uint32_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *ptr = (const uint8_t *)data;

    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= ptr[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return crc ^ 0xFFFFFFFF;
}

void FlsIf_Version_Init(void)
{
    rt_kprintf("MCU_UID:[");
    for(int i=0; i<3; i++)
    {
        rt_kprintf("%08x", readBufferUID[i]);
    }
    rt_kprintf("]\r\n");
    rt_kprintf("MCU_Name:[N32G052KBQ7]\r\n");
    rt_kprintf("codeSoftVersion: [PS-G1_1.1.3-Beta_20260420]\r\n");
#if (PRODUCT_TYPE == TYPE_2KW)
    rt_kprintf("Product_Type:[2+7KW]\r\n");
#endif
#if (PRODUCT_TYPE == TYPE_3_5KW)
    rt_kprintf("Product_Type:[3.5+7KW]\r\n");
#endif
    rt_kprintf("voltage_cail_k = [%d],voltage_cail_b = [%d]\r\n",ADCM_t.cali.voltage.cail_k,ADCM_t.cali.voltage.cail_b);
    rt_kprintf("current_cail_k = [%d],current_cail_b = [%d]\r\n",ADCM_t.cali.current.cail_k,ADCM_t.cali.current.cail_b);
}

/**********************************************************************
*函数名称 : void FlsIf_cali_voltage_Init(void)
*参数      : void
*返回值    : void
*描述      : Fls校准数据初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void FlsIf_cali_voltage_Init(void)
{
    static uint32_t readvolBufA[2];
    static uint32_t readvolBufB[2];
    static uint32_t readCrc[2]; /* *[0]=电压CRC, [1]=电流CRC */
    uint32_t calc_volcrc[2];

    FlsIf_ReadMultiple(CALIBRATION_VOL_A_ADDR, readvolBufA, 8);
    FlsIf_ReadMultiple(CALIBRATION_VOL_B_ADDR, readvolBufB, 8);
    FlsIf_ReadMultiple(CALIBRATION_CRC_ADDR, readCrc, 8);

    /* A区和B区数据相等，直接使用 */
    if((readvolBufA[0] == readvolBufB[0]) && (readvolBufA[1] == readvolBufB[1])
        && (readvolBufA[0] != 0xffffffff) && (readvolBufA[1] != 0xffffffff))
    {
        ADCM_t.cali.voltage.cail_k = readvolBufA[0];
        ADCM_t.cali.voltage.cail_b = readvolBufA[1];
        rt_kprintf("voltage_cail_k = [%d],voltage_cail_b = [%d]\r\n",ADCM_t.cali.voltage.cail_k,ADCM_t.cali.voltage.cail_b); // (A==B)
        return;
    }

    /* A区和B区不相等，分别校验CRC */
    /* 校验A区CRC */
    if((readvolBufA[0] != 0xffffffff) && (readvolBufA[1] != 0xffffffff))
    {
        calc_volcrc[0] = calibration_crc32(readvolBufA, 8);
        if(calc_volcrc[0] == readCrc[0])
        {
            ADCM_t.cali.voltage.cail_k = readvolBufA[0];
            ADCM_t.cali.voltage.cail_b = readvolBufA[1];
            FLASH_Unlock();
            /* 写入B区 */
            FlsIf_ErasePage(CALIBRATION_VOL_B_ADDR);
            FlsIf_ProgramMultiple(CALIBRATION_VOL_B_ADDR, readvolBufA, 8);
            FLASH_Lock();
            rt_kprintf("voltage_cail_k = [%d],voltage_cail_b = [%d]\r\n",ADCM_t.cali.voltage.cail_k,ADCM_t.cali.voltage.cail_b); // (A区CRC通过)
            return;
        }
    }

    /* A区CRC不通过，校验B区CRC */
    if((readvolBufB[0] != 0xffffffff) && (readvolBufB[1] != 0xffffffff))
    {
        calc_volcrc[1] = calibration_crc32(readvolBufB, 8);
        if(calc_volcrc[1] == readCrc[0])
        {
            ADCM_t.cali.voltage.cail_k = readvolBufB[0];
            ADCM_t.cali.voltage.cail_b = readvolBufB[1];
            FLASH_Unlock();
            /* 写入A区 */
            FlsIf_ErasePage(CALIBRATION_VOL_A_ADDR);
            FlsIf_ProgramMultiple(CALIBRATION_VOL_A_ADDR, readvolBufB, 8);
            FLASH_Lock();
            rt_kprintf("voltage_cail_k = [%d],voltage_cail_b = [%d]\r\n",ADCM_t.cali.voltage.cail_k,ADCM_t.cali.voltage.cail_b); // (B区CRC通过)
            return;
        }
    }

    /* A区B区CRC均不通过，使用默认值 */
    ADCM_t.cali.voltage.cail_k = 1000;
    ADCM_t.cali.voltage.cail_b = 0;
    rt_kprintf("voltage_cail_k = [%d],voltage_cail_b = [%d]\r\n",ADCM_t.cali.voltage.cail_k,ADCM_t.cali.voltage.cail_b);
}

/**********************************************************************
*函数名称 : void FlsIf_cali_current_Init(void)
*参数      : void
*返回值    : void
*描述      : Fls校准数据初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void FlsIf_cali_current_Init(void)
{
    static uint32_t readcurBufA[2];
    static uint32_t readcurBufB[2];
    static uint32_t readCrc[2]; /* *[0]=电压CRC, [1]=电流CRC */
    uint32_t calc_curcrc[2];

    FlsIf_ReadMultiple(CALIBRATION_CUR_A_ADDR, readcurBufA, 8);
    FlsIf_ReadMultiple(CALIBRATION_CUR_B_ADDR, readcurBufB, 8);
    FlsIf_ReadMultiple(CALIBRATION_CRC_ADDR, readCrc, 8);

    /* A区和B区数据相等，直接使用 */
    if((readcurBufA[0] == readcurBufB[0]) && (readcurBufA[1] == readcurBufB[1])
        && (readcurBufA[0] != 0xffffffff) && (readcurBufA[1] != 0xffffffff))
    {
        ADCM_t.cali.current.cail_k = readcurBufA[0];
        ADCM_t.cali.current.cail_b = readcurBufA[1];
        rt_kprintf("current_cail_k = [%d],current_cail_b = [%d]\r\n",ADCM_t.cali.current.cail_k,ADCM_t.cali.current.cail_b); // (A==B)
        return;
    }

    /* A区和B区不相等，分别校验CRC */
    /* 校验A区CRC */
    if((readcurBufA[0] != 0xffffffff) && (readcurBufA[1] != 0xffffffff))
    {
        calc_curcrc[0] = calibration_crc32(readcurBufA, 8);
        if(calc_curcrc[0] == readCrc[1])
        {
            ADCM_t.cali.current.cail_k = readcurBufA[0];
            ADCM_t.cali.current.cail_b = readcurBufA[1];
            FLASH_Unlock();
            /* 写入B区 */
            FlsIf_ErasePage(CALIBRATION_CUR_B_ADDR);
            FlsIf_ProgramMultiple(CALIBRATION_CUR_B_ADDR, readcurBufA, 8);
            FLASH_Lock();
            rt_kprintf("current_cail_k = [%d],current_cail_b = [%d]\r\n",ADCM_t.cali.current.cail_k,ADCM_t.cali.current.cail_b); // (A区CRC通过)
            return;
        }
    }

    /* A区CRC不通过，校验B区CRC */
    if((readcurBufB[0] != 0xffffffff) && (readcurBufB[1] != 0xffffffff))
    {
        calc_curcrc[1] = calibration_crc32(readcurBufB, 8);
        if(calc_curcrc[1] == readCrc[1])
        {
            ADCM_t.cali.current.cail_k = readcurBufB[0];
            ADCM_t.cali.current.cail_b = readcurBufB[1];
            FLASH_Unlock();
            /* 写入A区 */
            FlsIf_ErasePage(CALIBRATION_CUR_A_ADDR);
            FlsIf_ProgramMultiple(CALIBRATION_CUR_A_ADDR, readcurBufB, 8);
            FLASH_Lock();
            rt_kprintf("current_cail_k = [%d],current_cail_b = [%d]\r\n",ADCM_t.cali.current.cail_k,ADCM_t.cali.current.cail_b); // (B区CRC通过)
            return;
        }
    }

    /* A区B区CRC均不通过，使用默认值 */
    ADCM_t.cali.current.cail_k = 1000;
    ADCM_t.cali.current.cail_b = 0;
    rt_kprintf("current_cail_k = [%d],current_cail_b = [%d]\r\n",ADCM_t.cali.current.cail_k,ADCM_t.cali.current.cail_b);
}

/**********************************************************************
*函数名称 : void FlsIf_Init(void)
*参数      : void
*返回值    : void
*描述      : Fls接口初始化
*编辑时间  :
*备注      : 初版
***********************************************************************/
void FlsIf_Init(void)
{
    FlsIf_ReadMultiple(0x1FFFF910, readBufferUID, 16);
//  FlsIf_Version_Init();
    FlsIf_cali_voltage_Init();
    FlsIf_cali_current_Init();
}
