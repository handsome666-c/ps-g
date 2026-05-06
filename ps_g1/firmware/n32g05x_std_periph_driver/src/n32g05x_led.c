/**
*     Copyright (c) 2023, Nations Technologies Inc.
* 
*     All rights reserved.
*
*     This software is the exclusive property of Nations Technologies Inc. (Hereinafter 
* referred to as NATIONS). This software, and the product of NATIONS described herein 
* (Hereinafter referred to as the Product) are owned by NATIONS under the laws and treaties
* of the People's Republic of China and other applicable jurisdictions worldwide.
*
*     NATIONS does not grant any license under its patents, copyrights, trademarks, or other 
* intellectual property rights. Names and brands of third party may be mentioned or referred 
* thereto (if any) for identification purposes only.
*
*     NATIONS reserves the right to make changes, corrections, enhancements, modifications, and 
* improvements to this software at any time without notice. Please contact NATIONS and obtain 
* the latest version of this software before placing orders.

*     Although NATIONS has attempted to provide accurate and reliable information, NATIONS assumes 
* no responsibility for the accuracy and reliability of this software.
* 
*     It is the responsibility of the user of this software to properly design, program, and test 
* the functionality and safety of any application made of this information and any resulting product. 
* In no event shall NATIONS be liable for any direct, indirect, incidental, special,exemplary, or 
* consequential damages arising in any way out of the use of this software or the Product.
*
*     NATIONS Products are neither intended nor warranted for usage in systems or equipment, any
* malfunction or failure of which may cause loss of human life, bodily injury or severe property 
* damage. Such applications are deemed, "Insecure Usage".
*
*     All Insecure Usage shall be made at user's risk. User shall indemnify NATIONS and hold NATIONS 
* harmless from and against all claims, costs, damages, and other liabilities, arising from or related 
* to any customer's Insecure Usage.

*     Any express or implied warranty with regard to this software or the Product, including,but not 
* limited to, the warranties of merchantability, fitness for a particular purpose and non-infringement
* are disclaimed to the fullest extent permitted by law.

*     Unless otherwise explicitly permitted by NATIONS, anyone may not duplicate, modify, transcribe
* or otherwise distribute this software for any purposes, in whole or in part.
*
*     NATIONS products and technologies shall not be used for or incorporated into any products or systems
* whose manufacture, use, or sale is prohibited under any applicable domestic or foreign laws or regulations. 
* User shall comply with any applicable export control laws and regulations promulgated and administered by 
* the governments of any countries asserting jurisdiction over the parties or transactions.
**/

/**
*\*\file n32g05x_led.c
*\*\author Nations 
*\*\version v1.0.1
*\*\copyright Copyright (c) 2023, Nations Technologies Inc. All rights reserved.
**/

#include "n32g05x_led.h"
#include "n32g05x.h"

const uint32_t LED_TrimAddrFromNVR[]  = { NVR_ADDR_90H_TRIM,NVR_ADDR_91H_TRIM,NVR_ADDR_92H_TRIM,NVR_ADDR_93H_TRIM,\
                                          NVR_ADDR_94H_TRIM,NVR_ADDR_95H_TRIM,NVR_ADDR_96H_TRIM,NVR_ADDR_97H_TRIM };

const uint16_t COM_Number[] = {0x0000,0x0041,0x00C2,0x01C3,0x03C4,0x07C5,0x0FC6,0x1FC7,0x3FC8,0x7FC9,0xFFCA};

const GPIO_Module* SEG_PORT[] = {SEG0_2PORT, SEG0_2PORT, SEG0_2PORT, SEG3_PORT, SEG4_PORT, SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT, \
                                 SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT, SEG5_15_PORT };

const uint32_t SEG_PIN[] = {SEG0_PIN, SEG1_PIN, SEG2_PIN, SEG3_PIN, SEG4_PIN, SEG5_PIN, SEG6_PIN, SEG7_PIN, \
                            SEG8_PIN, SEG9_PIN, SEG10_PIN, SEG11_PIN, SEG12_PIN, SEG13_PIN, SEG14_PIN, SEG15_PIN };
                        

/**
*\*\name    SPI_SendDataToLED.
*\*\fun     Sends a half-word data through the SPI interface and return the data received from the SPI bus
*\*\param   data  
*\*\         - data to send
*\*\return  The value of the received data
**/
uint16_t SPI_SendDataToLED(uint16_t data)
{
    /* Loop while DAT register in not emplty */
   __IO uint32_t timeout  = 0xFFF;
    
    while((SPI_GetStatus(SPI_LED,SPI_TE_FLAG) == RESET)&& (timeout !=0))
    {
       timeout--;
    } ;
    SPI_TransmitData(SPI_LED,data);
    
    /* Send byte through the SPI1 peripheral */
    timeout = 0xFFF;
    while((SPI_GetStatus(SPI_LED,SPI_RNE_FLAG) == RESET)&& (timeout !=0))
    {
       timeout--;
    } ;

    /* Return the byte read from the SPI bus */
    return SPI_ReceiveData(SPI_LED);
}

/**
*\*\name    LED_TrimGet.
*\*\fun     Get LED trim from NVR.
*\*\return  none
**/
void LED_TrimGet(LED_InitType *led_status_get)
{
    uint32_t tmpreg  = 0;
    uint16_t index   = 0;

    for(index = 0;index<6;index++)
    {
        tmpreg = *(uint32_t*)LED_TrimAddrFromNVR[index];
        led_status_get->Trim_SEG[index] = (uint8_t)tmpreg;
    }
    tmpreg = *(uint32_t*)LED_TrimAddrFromNVR[6];
    led_status_get->Trim_Rset = (uint8_t)tmpreg;
    
    tmpreg = *(uint32_t*)LED_TrimAddrFromNVR[7];
    led_status_get->Trim_BG = (uint8_t)tmpreg;
}

/**
*\*\name    LED_TrimLoad.
*\*\fun     Get LED trim from NVR and set to LED.
*\*\return  none
**/
void LED_TrimLoad(LED_InitType *led_status_set)
{
    uint32_t tmpreg  = 0;
    uint16_t tmpreg1 = 0;
    uint16_t index   = 0;

    for(index = 0;index<6;index++)
    {
        tmpreg1 = 0;
        tmpreg = *(uint32_t*)LED_TrimAddrFromNVR[index];
        tmpreg1 = (uint8_t)tmpreg;
        led_status_set->Trim_SEG[index] = (uint8_t)tmpreg1;
        tmpreg1 |= (0x9000 + (0x100*index) );
        SPI_SendDataToLED(tmpreg1);
    }
    tmpreg1 = 0;
    tmpreg = *(uint32_t*)LED_TrimAddrFromNVR[6];
    tmpreg1 = (uint8_t)(tmpreg&0x1F);
    led_status_set->Trim_Rset = (uint8_t)tmpreg1;
    tmpreg1 |= (LED_CMD_RSET_TRIM_REG_WRITE);
    SPI_SendDataToLED(tmpreg1);
    
    tmpreg1 = 0;
    tmpreg = *(uint32_t*)LED_TrimAddrFromNVR[7];
    tmpreg1 = (uint8_t)(tmpreg&0x1F);
    led_status_set->Trim_BG = (uint8_t)tmpreg1;
    tmpreg1 |= (LED_CMD_BG_TRIM_REG_WRITE);
    SPI_SendDataToLED(tmpreg1);
}

/**
*\*\name    LED_DeInit.
*\*\fun     Reset LED peripheral.
*\*\return  none
*\*\note    Ensure that LED_Init has been called.
**/
void LED_DeInit(void)
{
    /* DISABLE Gclk Output */
    RCC_EnableLEDClk(DISABLE);
    /* config LED_CMD_COM_FORCE_SHUTDOWM register */
    SPI_SendDataToLED(LED_CMD_COM_FORCE_SHUTDOWM);
}

/**
*\*\name    LED_Init.
*\*\fun     Initializes the LED peripheral according to the specified parameters in the
*\*\        LED_InitStruct.
*\*\param   LED_InitStruct.
*\*\          - GCLK_LowLevelNum
*\*\          	- GCLK_LOWLEVELNUM_2 ~ GCLK_LOWLEVELNUM_255
*\*\          - COM_WorkNum
*\*\          	- LED_COM_WORKNUM_2	((uint8_t)	2U)
*\*\          	- LED_COM_WORKNUM_3	((uint8_t)	3U)
*\*\          	- LED_COM_WORKNUM_4	((uint8_t)	4U)
*\*\          	- LED_COM_WORKNUM_5	((uint8_t)	5U)
*\*\          	- LED_COM_WORKNUM_6	((uint8_t)	6U)
*\*\          	- LED_COM_WORKNUM_7	((uint8_t)	7U)
*\*\          	- LED_COM_WORKNUM_8	((uint8_t)	8U)
*\*\          	- LED_COM_WORKNUM_9	((uint8_t)	9U)
*\*\          	- LED_COM_WORKNUM_10	((uint8_t)	10U)
*\*\          - SEG_WorkCurrent
*\*\          	- LED_WORKCURRENT_5_MA
*\*\          	- LED_WORKCURRENT_5_P5_MA
*\*\          	- LED_WORKCURRENT_6_MA
*\*\          	- LED_WORKCURRENT_6_P5_MA
*\*\          	- LED_WORKCURRENT_7_MA
*\*\          	- LED_WORKCURRENT_7_P5_MA
*\*\          	- LED_WORKCURRENT_8_MA
*\*\          	- LED_WORKCURRENT_8_P5_MA
*\*\          	- LED_WORKCURRENT_9_MA
*\*\          	- LED_WORKCURRENT_9_P5_MA
*\*\          	- LED_WORKCURRENT_10_MA
*\*\          	- LED_WORKCURRENT_10_P5_MA
*\*\          	- LED_WORKCURRENT_11_MA
*\*\          	- LED_WORKCURRENT_11_P5_MA
*\*\          	- LED_WORKCURRENT_12_MA
*\*\          	- LED_WORKCURRENT_12_P5_MA
*\*\          	- LED_WORKCURRENT_13_MA
*\*\          	- LED_WORKCURRENT_13_P5_MA
*\*\          	- LED_WORKCURRENT_14_MA
*\*\          	- LED_WORKCURRENT_14_P5_MA
*\*\          	- LED_WORKCURRENT_15_MA
*\*\          	- LED_WORKCURRENT_15_P5_MA
*\*\          	- LED_WORKCURRENT_16_MA
*\*\          	- LED_WORKCURRENT_16_P5_MA
*\*\          	- LED_WORKCURRENT_17_MA
*\*\          	- LED_WORKCURRENT_17_P5_MA
*\*\          	- LED_WORKCURRENT_18_MA
*\*\          	- LED_WORKCURRENT_18_P5_MA
*\*\          	- LED_WORKCURRENT_19_MA
*\*\          	- LED_WORKCURRENT_19_P5_MA
*\*\          	- LED_WORKCURRENT_20_MA
*\*\          - ScanMode
*\*\          	- LED_SCAN_MODE_8SEG_8SEG
*\*\          	- LED_SCAN_MODE_16SEG
*\*\          - SEG_WorkStatus
*\*\          	- LED_WORK_MODE_DISABLE
*\*\          	- LED_WORK_MODE_ENABLE
*\*\return  none
**/
void LED_Init(LED_InitType* LED_InitStruct)
{
    uint16_t tmpreg1 = 0;
    
    /* config LED_CMD_COM_FORCE_SHUTDOWM register */
    SPI_SendDataToLED(LED_CMD_COM_FORCE_SHUTDOWM);

    SPI_SendDataToLED(LED_CMD_COM_SCAN_RESET);
    SPI_SendDataToLED(0xA000U);
	
    /* config LED_CMD_CHIP_WAKEUP register */
    SPI_SendDataToLED(LED_CMD_CHIP_WAKEUP);
    /* config LED_CMD_COM_NUM_COM0_1_WORK_STATUS register */
    tmpreg1 |= LED_CMD_COM_NUM_COM0_1_WORK_STATUS;
    tmpreg1 |= (COM_Number[LED_InitStruct->COM_WorkNum]&0x00FFU);
    SPI_SendDataToLED(tmpreg1);
    tmpreg1 = 0;
    /* config LED_CMD_COM2_9_WORK_STATUS register */
    tmpreg1 |= LED_CMD_COM2_9_WORK_STATUS;
    tmpreg1 |= COM_Number[LED_InitStruct->COM_WorkNum]>>8;
    SPI_SendDataToLED(tmpreg1);
	
    tmpreg1 = 0;
    /* config LED_CMD_SEG0_7_WORK_STATUS register */
    /* set seg0 ~ seg7 work status  */
    tmpreg1 |= LED_CMD_SEG0_7_WORK_STATUS;
    tmpreg1 |= (LED_InitStruct->SEG_WorkStatus&0x00ffU);
    SPI_SendDataToLED(tmpreg1);
    tmpreg1 = 0;
    /* set seg8 ~ seg15 work status  */
    tmpreg1 |= LED_CMD_SEG8_15_WORK_STATUS;
    tmpreg1 |= (LED_InitStruct->SEG_WorkStatus>>8);
    SPI_SendDataToLED(tmpreg1);
	
	
    tmpreg1 = 0;
    /* config LED_CMD_SEG_CUR_DISP_MODE_CHIP_EN register */
    tmpreg1 |= LED_CMD_SEG_CUR_DISP_MODE_CHIP_EN;
    /* set work current */
    tmpreg1 |= (uint16_t)LED_InitStruct->SEG_WorkCurrent | 0x0020U;
    /* set scan mode */
    tmpreg1 |= ((uint16_t)LED_InitStruct->Scan_Mode<<6);
		/* set work mode */
    tmpreg1 |= ((uint16_t)LED_InitStruct->Work_Mode)<<7;
    SPI_SendDataToLED(tmpreg1);
	
    tmpreg1 = 0;
    /* Load the trim value from NVR address for LED*/
    LED_TrimLoad(LED_InitStruct);
}

/**
*\*\name    LED_SEG_DutyDataSet.
*\*\fun     Set the LED seg duty data.
*\*\param   SegId .
*\*\          - this value must be range of 0~15
*\*\param   duty .
*\*\          - this value must be range of 0~63
*\*\return  none
**/
void LED_SEG_DutyDataSet(uint8_t SegId, uint8_t duty)
{
    uint16_t tmpreg1;

    if(SegId < MAX_LED_SEG_ID)
    {
        tmpreg1  = (LED_CMD_SEG0_DUTY_DATA+ (SegId *0x100)) ;
        if(duty < MAX_LED_SEG_DUTY)
        {
            tmpreg1 |= ((uint16_t)duty & MAX_LED_SEG_DUTY);
        }
        else
        {
            tmpreg1 |= MAX_LED_SEG_DUTY;
        }
        SPI_SendDataToLED(tmpreg1);
    }
    else
    {
        /* no process */
    }
}
/**
*\*\name    LED_Get_Cur_ScanCOM_List.
*\*\fun     Get the current scanning COM list for LED.
*\*\param   COM_num:total numbers of LED operation;
*\*\param   index:
*\*\param   com_scan_list:  history list of COM port scans. 
*\*\return  none.
**/
void LED_Get_Cur_ScanCOM_List(uint8_t COM_num, uint8_t index, uint8_t *com_scan_list)
{
    uint16_t tmpreg1,tmpreg2;

    if(index >= COM_num)
    {
        return;
    }
    
    SPI_LED->DAT;
    SPI_LED->DAT;
    
    tmpreg1 = LED_CMD_READ_REG;
    tmpreg1 |= 0x27U;
    tmpreg1 = SPI_SendDataToLED(tmpreg1);

    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
    
    com_scan_list[index &0x0F] = (uint8_t)(tmpreg2&0xFFU);

    /* Close the read back command */
    SPI_SendDataToLED(LED_CMD_READ_OFF);
}

/**
*\*\name    LED_Check_Cur_ScanCOM.
*\*\fun     Check the current scanning COM list for LED.
*\*\param   COM_num:total numbers of LED operation;
*\*\param   index:
*\*\return  none.
**/
ErrorStatus LED_Check_Cur_ScanCOM(uint8_t COM_num, uint8_t index)
{
    uint16_t tmpreg1,tmpreg2;

    if(index >= COM_num)
    {
        return ERROR;
    }
    
    SPI_LED->DAT;
    SPI_LED->DAT;
    
    tmpreg1 = LED_CMD_READ_REG;
    tmpreg1 |= 0x27U;
    tmpreg1 = SPI_SendDataToLED(tmpreg1);

    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);

    /* Close the read back command */
    SPI_SendDataToLED(LED_CMD_READ_OFF);
    
    if((uint8_t)(tmpreg2&0xFFU) != index)
    {
        return ERROR;
    }
    else
    {
       /* no process */
    }
    
    return SUCCESS;
}
/**
*\*\name    Get_LED_Scan_COM_Status.
*\*\fun     Get the current scanning COM status.
*\*\param   COM_num:total numbers of LED operation;
*\*\param   com_scan_list:  history list of COM port scans. 
*\*\return  ErrorStatus. 
*\*\          - ERROR:  LED operation status is abnormal;
*\*\          - SUCCESS:  LED operation status is normal;
**/
ErrorStatus Get_LED_Scan_COM_Status(uint8_t COM_num, uint8_t *com_scan_list)
{
    uint8_t index;
    ErrorStatus status = SUCCESS ;
    
    for(index = 0; index < COM_num; index++)
    {
        if (index != com_scan_list[index])
        {
            status = ERROR;
        }
    }
    return status;
}

/**
*\*\name    LED_Get_RegData.
*\*\fun     Get the status of LED,including triming data, work current, ect.
*\*\param   none.
*\*\return  none.
**/
void LED_Get_RegData(LED_InitType *led_status_get)
{
    uint16_t tmpreg1, tmpreg2;
    uint16_t i;

    /* Read back the RSET command */
    tmpreg1 = LED_CMD_READ_REG;
    tmpreg1 |= 0x16U;
    SPI_SendDataToLED(tmpreg1) ;
    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
    led_status_get->Trim_Rset = (uint8_t)(tmpreg2&0x1FU) ;
    tmpreg1 = 0;
    
    /* Read back the BG command */
    tmpreg1 |= LED_CMD_READ_REG;
    tmpreg1 |= 0x17U;
    SPI_SendDataToLED(tmpreg1);
    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
    led_status_get->Trim_BG = (uint8_t)(tmpreg2&0x1FU);
    tmpreg1 = 0;  
    
    /* Read back the Trim duty commands */
    for(i = 0;i<6;i++)
    {
        tmpreg1 |= LED_CMD_READ_REG;
        tmpreg1 |= 0x10 + i;
        SPI_SendDataToLED(tmpreg1);
        tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
        led_status_get->Trim_SEG[i] = (uint8_t)(tmpreg2 &0xFFU);
        tmpreg1 = 0;
    }
    
    /* Read back the work current / display mode/ chip enable commands */
    tmpreg1 |= LED_CMD_READ_REG;
    tmpreg1 |= 0x20U;
    SPI_SendDataToLED(tmpreg1) ;
    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
    led_status_get->SEG_WorkCurrent = (tmpreg2 &0x1FU);
    led_status_get->Scan_Mode =  ((tmpreg2 & 0x0040U) != 0)? 0x01U : 0x00U ;
    led_status_get->Work_Mode = ((tmpreg2 & 0x80U) != 0)? 0x01U : 0x00U ;
    tmpreg1 = 0; 
    
    /* Read back the the number of COM commands */
    tmpreg1 |= LED_CMD_READ_REG;
    tmpreg1 |= 0x21U;
    SPI_SendDataToLED(tmpreg1);
    SPI_LED->DAT;
    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);;
    led_status_get->COM_WorkNum = (uint8_t)(tmpreg2 &0x0FU) ;
    tmpreg1 = 0;
    
    /* Read back the the working status of SEG0 -SEG7 commands */
    tmpreg1 |= LED_CMD_READ_REG;
    tmpreg1 |= 0x23U;
    SPI_SendDataToLED(tmpreg1);
    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);;
    led_status_get->SEG_WorkStatus = (uint8_t)(tmpreg2&0xFFU) ;
    tmpreg1 = 0;
    
    /* Read back the the working status of SEG8 -SEG15 commands */
    tmpreg1 |= LED_CMD_READ_REG;
    tmpreg1 |= 0x24U;
    SPI_SendDataToLED(tmpreg1);
    tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
    led_status_get->SEG_WorkStatus |= ((tmpreg2&0xFFU)<<8) ;
    tmpreg1 = 0;
    
    /* Read back the the duty status of SEG0 -SEG15 */
    for(i = 0;i<16;i++)
    {
        tmpreg1 |= LED_CMD_READ_REG;
        tmpreg1 |= 0x30U + i;
        SPI_SendDataToLED(tmpreg1);
        tmpreg2= SPI_SendDataToLED(SPI_DUMMY_DATA);
        led_status_get->SEG_Duty[i] = (uint8_t)(tmpreg2&0xFFU);
        tmpreg1 = 0;
    }

    /* Close the read back command */
    SPI_SendDataToLED(LED_CMD_READ_OFF);
}
/**
*\*\name    LED_InitStruct.
*\*\fun     Fills each LED_InitType member with its default value.
*\*\param   LED_InitType (The input parameters must be the following values):
*\*\          - GCLK_LowLevelNum
*\*\          - COM_WorkNum
*\*\          - SEG_WorkCurrent
*\*\          - SEG_WorkStatus
*\*\          - Scan_Mode
*\*\          - Work_Mode
*\*\return  none
**/
void LED_InitStruct(LED_InitType* LED_InitStruct)
{
    uint8_t index = 0U;
	
    /* initialize the GCLK_LowLevelNum member */
    LED_InitStruct->GCLK_LowLevelNum        = GCLK_LOWLEVELNUM_20;
    /* Initialize the COM_WorkNum member */
    LED_InitStruct->COM_WorkNum             = LED_COM_WORKNUM_10;
    /* initialize the SEG_WorkCurrent member */
    LED_InitStruct->SEG_WorkCurrent         = LED_WORKCURRENT_7_P5_MA;
    /* Initialize the SEG_WorkStatus */
    LED_InitStruct->SEG_WorkStatus          = 0U;
    /* Initialize the ScanMode */
    LED_InitStruct->Scan_Mode               = LED_SCAN_MODE_8SEG_8SEG;
    /* Initialize the LED status */
    LED_InitStruct->Work_Mode               = DISABLE;

    /* Initialize the LED Cur_ScanCOM */
    LED_InitStruct->Cur_ScanCOM             = LED_CUR_SCANCOM_SHUTDOWM;
    /* Initialize the LED SEG_Duty */
    for(index = 0; index < MAX_LED_SEG_ID; index++)
    {
        LED_InitStruct->SEG_Duty[index]     = 0U;
    }
    
    /* Initialize the LED Trim_Rset */
    LED_InitStruct->Trim_Rset               = 0U;
    /* Initialize the LED Trim_BG */
    LED_InitStruct->Trim_BG                 = 0U;
    /* Initialize the LED Trim_SEG */
    for(index = 0; index < 6U; index++)
    {
        LED_InitStruct->Trim_SEG[index]     = 0U;
    }
}

/**
*\*\name    LED_GclkConfiguration.
*\*\fun     Configures GCLK clocks frequency and init configuration.
*\*\param   LED_clksrc(LED clock source):
*\*\         - LED_GCLK_SRC_HSI    HSI clock selected 
*\*\         - LED_GCLK_SRC_HSE    HSE clock selected 
*\*\param   LED_prescaler(LED clock prescaler):
*\*\         - LED_GCLK_DIV1 
*\*\         - LED_GCLK_DIV32
*\*\         - LED_GCLK_DIV40
*\*\         - LED_GCLK_DIV50
*\*\         - LED_GCLK_DIV64
*\*\         - LED_GCLK_DIV80
*\*\         - LED_GCLK_DIV160
*\*\return  none
*\*\note  LED GCLK must be in the range of 50kHz ~ 250kHz.
**/
void LED_GclkConfiguration(uint32_t LED_clksrc, uint32_t LED_prescaler)
{
    /* Configures the LED clock frequency */
    RCC_ConfigLEDClk(LED_clksrc,LED_prescaler);
    /* this value must be fix at 130 */
    RCC_SetLEDClockCycles(LED_CLOCK_CYCLE_NUMBER);
}

/**
*\*\name    LED_SPI_Init.
*\*\fun     Configures the different system clocks.
*\*\return  none
*\*\note  LED SPI Baud-rate cannot greater than 3M.
**/
void LED_SpiInit(void)
{
    SPI_InitType SPI_InitStructure;

    SPI_DeInit(SPI_LED);

    SPI_InitStruct(&SPI_InitStructure);
    SPI_InitStructure.DataDirection = SPI_DIR_DOUBLELINE_FULLDUPLEX;   
    SPI_InitStructure.SpiMode       = SPI_MODE_MASTER;
    SPI_InitStructure.DataLen       = SPI_DATA_SIZE_16BITS;
    SPI_InitStructure.CLKPOL        = SPI_CLKPOL_LOW;
    SPI_InitStructure.CLKPHA        = SPI_CLKPHA_FIRST_EDGE;
    SPI_InitStructure.NSS           = SPI_NSS_HARD; 
    SPI_InitStructure.BaudRatePres  = SPI_BR_PRESCALER_32;
    SPI_InitStructure.FirstBit      = SPI_FB_MSB;
    SPI_InitStructure.CRCPoly       = 7;
    SPI_Init(SPI_LED, &SPI_InitStructure);
    
    SPI_SSOutputEnable(SPI_LED, ENABLE);

    SPI_Enable(SPI_LED, ENABLE);
}

/**
*\*\name    RCC_Configuration.
*\*\fun     Configures the different system clocks.
*\*\return  none
**/
void LED_RccConfiguration(void)
{
    /* Enable LED_SPIx / LED_SPIx_GPIO / GCLK GPIO / LED_COM_SEG GPIO clocks */
    RCC_EnableAPB2PeriphClk(SPIx_CLK | SPIx_LED_CLK | RCC_APB2_PERIPH_AFIO | \
	                          RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | \
	                          RCC_APB2_PERIPH_GPIOC | RCC_APB2_PERIPH_GPIOD, ENABLE);
}

/**
*\*\name    LED_GpioConfiguration.
*\*\fun     Configures the different SPIx GPIO, GCLK ports, LED COM and SEG ports according to the specified params COM_WorkNum and SEG_WorkStatus.
*\*\param   COM_WorkNum. the total number of working COM.  
*\*\          -  this value must be range of 1 ~ 10.
*\*\param   SEG_WorkStatus. selected the working SEG.
*\*\          -  this value must be range of 0 ~ 0xFFFF.Bit0 - SEG0,Bit1 - SEG1...Bit15 - SEG15
*\*\             eg :when SEG_WorkStatus = 0x1841 it means SEG0,SEG6,SEG11,SEG15 will eable to work
*\*\return  none
**/
void LED_GpioConfiguration(uint8_t COM_WorkNum, uint16_t SEG_WorkStatus)
{
    GPIO_InitType GPIO_InitStructure;
    uint8_t  index ;

    GPIO_InitStruct(&GPIO_InitStructure);
    /* Configure Gclk pins: GCLK --------------------------------*/
    GPIO_InitStructure.Pin        = SPIx_LED_GCLK_PIN ;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_LED;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
    GPIO_InitPeripheral(SPIx_LED_GCLK_PORT, &GPIO_InitStructure);

    /* Configure SPIx pins: NSS ---------------------------------*/
    GPIO_InitStructure.Pin        = SPIx_LED_NSS_PIN ;
    GPIO_InitStructure.GPIO_Alternate = SPIx_LED_NSS_AFIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
    GPIO_InitPeripheral(SPIx_LED_NSS_PORT, &GPIO_InitStructure);

    /* Configure SPIx pins: SCK ---------------------------------*/
    GPIO_InitStructure.Pin        = SPIx_LED_SCK_PIN ;
    GPIO_InitStructure.GPIO_Alternate = SPIx_LED_SCK_AFIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
    GPIO_InitPeripheral(SPIx_LED_SCK_PORT, &GPIO_InitStructure);

    /* Configure SPIx pins: MOSI --------------------------------*/
    GPIO_InitStructure.Pin        = SPIx_LED_MOSI_PIN ;
    GPIO_InitStructure.GPIO_Alternate = SPIx_LED_MOSI_AFIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
    GPIO_InitPeripheral(SPIx_LED_MOSI_PORT, &GPIO_InitStructure);

    /* Configure SPIx pins: MISO -------------------------------*/
    GPIO_InitStructure.Pin        = SPIx_LED_MISO_PIN ;
    GPIO_InitStructure.GPIO_Alternate = SPIx_LED_MISO_AFIO;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull = GPIO_NO_PULL;
    GPIO_InitPeripheral(SPIx_LED_MISO_PORT, &GPIO_InitStructure);

    /* Configure COM pins---------------------------------------*/
    GPIO_InitStruct(&GPIO_InitStructure);
    if(COM_WorkNum >= 1)
    {
        GPIO_InitStructure.Pin        = COM0_PIN ;
        GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
        GPIO_InitPeripheral(COM0_2_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 2)
    {
        GPIO_InitStructure.Pin        = COM1_PIN ;
        GPIO_InitPeripheral(COM0_2_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 3)
    {
        GPIO_InitStructure.Pin        = COM2_PIN ;
        GPIO_InitPeripheral(COM0_2_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 4)
    {
        GPIO_InitStructure.Pin        = COM3_PIN ;
        GPIO_InitPeripheral(COM3_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 5)
    {
        GPIO_InitStructure.Pin        = COM4_PIN ;
        GPIO_InitPeripheral(COM4_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 6)
    {
        GPIO_InitStructure.Pin        = COM5_PIN ;
        GPIO_InitPeripheral(COM5_9_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 7)
    {
        GPIO_InitStructure.Pin        = COM6_PIN ;
        GPIO_InitPeripheral(COM5_9_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 8)
    {
        GPIO_InitStructure.Pin        = COM7_PIN ;
        GPIO_InitPeripheral(COM5_9_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 9)
    {
        GPIO_InitStructure.Pin        = COM8_PIN ;
        GPIO_InitPeripheral(COM5_9_PORT, &GPIO_InitStructure);
    }
    if(COM_WorkNum >= 10)
    {
        GPIO_InitStructure.Pin        = COM9_PIN ;
        GPIO_InitPeripheral(COM5_9_PORT, &GPIO_InitStructure);
    }
    /* Configure SEG pins---------------------------------------*/
    for(index = 0;index < 16;index ++)
    {
        if((SEG_WorkStatus & ((uint16_t)1<<index)) !=0 )
        {
            GPIO_InitStructure.Pin = SEG_PIN[index];
            GPIO_InitPeripheral((GPIO_Module*)SEG_PORT[index], &GPIO_InitStructure);
        }
    }
}
