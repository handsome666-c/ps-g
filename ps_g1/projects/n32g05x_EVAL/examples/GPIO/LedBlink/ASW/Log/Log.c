#include "log.h"

calibration_Manager_t calibration_Manager;

#if LOG_ENABLE

#define LOG_UARTx        UART1
#define LOG_PERIPH       RCC_APB2_PERIPH_UART1
#define LOG_GPIO         GPIOA
#define LOG_PERIPH_GPIO  RCC_APB2_PERIPH_GPIOA
#define LOG_TX_PIN       GPIO_PIN_9
#define LOG_RX_PIN       GPIO_PIN_10

void log_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    UART_InitType USART_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    RCC_EnableAPB2PeriphClk(LOG_PERIPH | RCC_APB2_PERIPH_AFIO | LOG_PERIPH_GPIO, ENABLE);

    NVIC_Configuration(UART1_IRQn, 3, ENABLE);

    GPIO_InitStructure.Pin             = LOG_TX_PIN;
    GPIO_InitStructure.GPIO_Pull       = GPIO_PULL_UP;
    GPIO_InitStructure.GPIO_Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_UART1;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.Pin             = LOG_RX_PIN;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_UART1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    UART_StructInit(&USART_InitStructure);
    USART_InitStructure.BaudRate       = 115200;
    USART_InitStructure.WordLength     = UART_WL_8B;
    USART_InitStructure.StopBits       = UART_STPB_1;
    USART_InitStructure.Parity         = UART_PE_NO;
    USART_InitStructure.Mode           = UART_MODE_RX | UART_MODE_TX;

    // init uart
    UART_Init(LOG_UARTx, &USART_InitStructure);

    /* Enable UARTz Receive and Transmit interrupts */
    // UART_ConfigInt(LOG_UARTx, UART_INT_RXDNE, ENABLE);
    // UART_ConfigInt(LOG_UARTx, UART_INT_TXDE, ENABLE);

    // enable uart
    UART_Enable(LOG_UARTx, ENABLE);
}

static int is_lr_sent = 0;

int fputc(int ch, FILE* f)
{
    if (ch == '\r')
    {
        is_lr_sent = 1;
    }
    else if (ch == '\n')
    {
        if (!is_lr_sent)
        {
            /* Loop until the end of transmission */
            while (UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXC) == RESET)
            {
            }
            UART_SendData(LOG_UARTx, (uint8_t)'\r');
        }
        is_lr_sent = 0;
    }
    else
    {
        is_lr_sent = 0;
    }

    /* Loop until the end of transmission */
    while (UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXC) == RESET)
    {
    }
    UART_SendData(LOG_UARTx, (uint8_t)ch);
    return ch;
}

#endif // LOG_ENABLE

/**
* @brief 1s周期打印信息函数
*
*/
void Log_Printf_Main(void)
{
    FlsIf_Version_Init(); // UID、MCU_Name、codesoftversion、volcalik、volcalib、curcalik、curcalib
    rt_kprintf(" >>>Sys_State = [%d],{-0 charge, -1:discharge}\r\n", getSysState());
    rt_kprintf(" >>>CP_State = [%d] {-0 idle, -1 connect, -2 EV_ready, -3 charging, -4 READY,-5 error}\r\n",APP_CP_GetCurrentStatus());
    rt_kprintf(" >>>CP_Voltage = [%d]V\r\n",RTE_Call_AdcM_Vlotage(ADCMCFG_CP_CH));
    rt_kprintf(" >>>Voltage_x1000 = [%d]mV\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_RMSVOLX1000));
    rt_kprintf(" >>>Voltage_cali_x1000 = [%d]mV\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_Voltage_CH));
    rt_kprintf(" >>>Current_x1000 = [%d]mA\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_RMSCURX1000));
    rt_kprintf(" >>>Current_cali_x1000 = [%d]mA\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_Currnet_CH));
    rt_kprintf(" >>>Relay_State = [%d],{-0 disconnect,-1 connect}\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_RelayCHK_CH));
    rt_kprintf(" >>>Diode_Voltage = [%d],{-0 Normal:,-1 Error}\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_EvdiodeCHK_CH));
    rt_kprintf(" >>>MCUTemp_val_x100 = [%d]°C\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_MCUTemp_CH));
    rt_kprintf(" >>>NTC1_volx1000 = [%d]V\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_NTC1_CH));
    rt_kprintf(" >>>NTC1Temp_val = [%d]°C\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_NTC1Temp_CH));
    rt_kprintf(" >>>Leakage_State = [%d],{-0 Normal,-1 Error}\r\n", RTE_Call_DioIf_ReadChannel(HW_DIO_CH_S_OUT));
    rt_kprintf(" >>>PGD_State = [%d],{-0 Normal:,-1 Error}\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_Ground_CH));
    rt_kprintf(" >>>ShortCircuit_State = [%d],{-0 Normal,-1 Error}\r\n", RTE_Call_AdcM_Vlotage(ADCMCFG_Shortcircuit_CH));

    if(getSysState() == 0)
    {
        rt_kprintf(" >>>CHG_LED_State = [%d],{-0 selfcheck, -1 idle, -2 connect, -3 charging, -4 pause, -5 error}\r\n", APP_CHG_GetStatusLED());
        rt_kprintf(" >>>CHG_Error_type = [%d]\r\n", APP_CHG_GetFault());
        rt_kprintf(" >>>Adapter_detection_current = [%d]\r\n", Get_Adapter_detection_current(&st_Adapter_state_machine));
    }
    else if(getSysState() == 1)
    {
        rt_kprintf(" >>>DISCHG_LED_State = [%d],{-6 discharge waiting, -7 discharging, -8 error}\r\n", APP_DISCHG_GetStatusLED());
        rt_kprintf(" >>>DISCHG_Error_type = [%d]\r\n", APP_DISCHG_GetFault());
    }
}

/**
* @brief 注入式命令行控制函数
*
*/
static void inject(int argc, char**argv)
{
    uint32_t  value =0;
    uint8_t  flag =0;
    if(memcmp(argv[1],"help",strlen("help")) == 0)
    {
        rt_kprintf("example: inject $module $value $flag\r\n");
        rt_kprintf("module: [cp,voltage,current,NTC1Temp,PGD,diode,relaychk,MCUTemp]\r\n");
        rt_kprintf("inject cp       $value*1000 $flag  ->inject cp     1200  1\r\n");// 12V
        rt_kprintf("inject voltage  $value*1000 $flag  ->inject voltage 220000 1\r\n");// 220V
        rt_kprintf("inject current  $value*1000 $flag  ->inject current  8000  1\r\n");// 8A
        rt_kprintf("inject diode    $value*100 $flag   ->inject diode  1200  1\r\n");// 12V
        rt_kprintf("inject mcutemp  $value*100 $flag   ->inject mcutemp 2500  1\r\n");// 25℃
        rt_kprintf("inject ntcltemp $value-25 $flag    ->inject ntcltemp 25    1\r\n");// 25℃-25
        rt_kprintf("inject pgd      $value $flag        ->inject pgd      0     1\r\n");// 0\1
        rt_kprintf("inject relaychk $value $flag        ->inject relaychk 0     1\r\n");// 0\1
        rt_kprintf("inject leakage  $value $flag        ->inject leakage 0     1\r\n");// 0\1
    }
    if(memcmp(argv[1],"cp",strlen("cp")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.cp.Value = (uint32_t)value;
        ADCM_t.debug.cp.flag = flag;
        rt_kprintf("inject-cp value*100:[%d] flag:[%d]\r\n",ADCM_t.debug.cp.Value,ADCM_t.debug.cp.flag);
    }
    if(memcmp(argv[1],"voltage",strlen("voltage")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.voltage.Value = (uint32_t)value;
        ADCM_t.debug.voltage.flag = flag;
        rt_kprintf("inject-voltage value:[%d] flag:[%d]\r\n",ADCM_t.debug.voltage.Value,ADCM_t.debug.voltage.flag);
    }
    if(memcmp(argv[1],"current",strlen("current")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.current.Value = (uint32_t)value;
        ADCM_t.debug.current.flag = flag;
        rt_kprintf("inject-current value:[%d] flag:[%d]\r\n",ADCM_t.debug.current.Value,ADCM_t.debug.current.flag);
    }
    if(memcmp(argv[1],"diode",strlen("diode")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.evdio.Value = (uint32_t)value;
        ADCM_t.debug.evdio.flag = flag;
        rt_kprintf("inject-diode value:[%d] flag:[%d]\r\n",ADCM_t.debug.evdio.Value,ADCM_t.debug.evdio.flag);
    }
    if(memcmp(argv[1],"mcutemp",strlen("mcutemp")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.mcutemp.Value = (uint32_t)value;
        ADCM_t.debug.mcutemp.flag = flag;
        rt_kprintf("inject-diode value:[%d] flag:[%d]\r\n",ADCM_t.debug.mcutemp.Value,ADCM_t.debug.mcutemp.flag);
    }
    if(memcmp(argv[1],"ntcltemp",strlen("ntcltemp")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.ntc1temp.Value = (uint32_t)value;
        ADCM_t.debug.ntc1temp.flag = flag;
        rt_kprintf("inject-diode value:[%d] flag:[%d]\r\n",ADCM_t.debug.ntc1temp.Value,ADCM_t.debug.ntc1temp.flag);
    }
    if(memcmp(argv[1],"pgd",strlen("pgd")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.ground.pgdstatus = (uint32_t)value;
        ADCM_t.debug.ground.flag = flag;
        rt_kprintf("inject-pgd value:[%d] flag:[%d]\r\n",ADCM_t.debug.ground.pgdstatus,ADCM_t.debug.ground.flag);
    }
    if(memcmp(argv[1],"relaychk",strlen("relaychk")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.rlychk.rlychkstatus = (uint32_t)value;
        ADCM_t.debug.rlychk.flag = flag;
        rt_kprintf("inject-relaychk value:[%d] flag:[%d]\r\n",ADCM_t.debug.rlychk.rlychkstatus,ADCM_t.debug.rlychk.flag);
    }
    if(memcmp(argv[1],"leakage",strlen("leakage")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.leakage.leakagestatus = (uint32_t)value;
        rt_kprintf("inject-leakage value:[%d] flag:[%d]\r\n",ADCM_t.debug.leakage.leakagestatus,ADCM_t.debug.leakage.flag);
    }
    if(memcmp(argv[1],"shortcircuit",strlen("shortcircuit")) == 0)
    {
        value = atoi(argv[2]);
        flag  = atoi(argv[3]);
        ADCM_t.debug.shortcircuit.shortcircuitstatus = (uint32_t)value;
        ADCM_t.debug.shortcircuit.flag = flag;
        rt_kprintf("inject-shortcircuit value:[%d] flag:[%d]\r\n",ADCM_t.debug.shortcircuit.shortcircuitstatus,ADCM_t.debug.shortcircuit.flag);
    }
}
MSH_CMD_EXPORT(inject , ...);

/**
* @brief 校准模式下，设置电压校准参数（A/B双备份 + CRC）
*
*/
void calibration_vol_set(calibration_Manager_t *calibration_Manager,int32_t k,int32_t b)
{
    uint32_t crc_buf[2];
    ADCM_t.cali.voltage.buff[0] = (uint32_t)k;
    ADCM_t.cali.voltage.buff[1] = (uint32_t)b;
    rt_kprintf("cali set vol:y =%dx + %d]\r\n", (int32_t)ADCM_t.cali.voltage.buff[0], (int32_t)ADCM_t.cali.voltage.buff[1] );
    FLASH_Unlock();
    /* 写入A区 */
    FlsIf_ErasePage(CALIBRATION_VOL_A_ADDR);
    FlsIf_ProgramMultiple(CALIBRATION_VOL_A_ADDR,ADCM_t.cali.voltage.buff,8);
    /* 写入B区 */
    FlsIf_ErasePage(CALIBRATION_VOL_B_ADDR);
    FlsIf_ProgramMultiple(CALIBRATION_VOL_B_ADDR,ADCM_t.cali.voltage.buff,8);
    /* 计算CRC并存储: crc_buf[0]=电压CRC, crc_buf[1]=电流CRC */
    crc_buf[0] = 0xFFFFFFFF;
    FlsIf_ReadMultiple(CALIBRATION_CRC_ADDR, crc_buf, 8);
    crc_buf[0] = calibration_crc32(ADCM_t.cali.voltage.buff, 8);
    FlsIf_ErasePage(CALIBRATION_CRC_ADDR);
    FlsIf_ProgramMultiple(CALIBRATION_CRC_ADDR, crc_buf, 8);
    FLASH_Lock();
    FlsIf_cali_voltage_Init();
}

/**
* @brief 校准模式下，清除电压校准参数
*
*/
void calibration_vol_clear(calibration_Manager_t *calibration_Manager)
{
    FLASH_Unlock();
    FlsIf_ErasePage(CALIBRATION_VOL_A_ADDR);
    FlsIf_ErasePage(CALIBRATION_VOL_B_ADDR);
    FlsIf_ErasePage(CALIBRATION_CRC_ADDR);
    FLASH_Lock();
    FlsIf_cali_voltage_Init();
}

/**
* @brief 校准模式下，设置电流校准参数（A/B双备份 + CRC）
*
*/
void calibration_cur_set(calibration_Manager_t *calibration_Manager,int32_t k,int32_t b)
{
    uint32_t crc_buf[2];
    ADCM_t.cali.current.buff[0] = (uint32_t)k;
    ADCM_t.cali.current.buff[1] = (uint32_t)b;
    rt_kprintf("cali set cur:y =%dx + %d]\r\n", (int32_t)ADCM_t.cali.current.buff[0], (int32_t)ADCM_t.cali.current.buff[1]);
    FLASH_Unlock();
    /* 写入A区 */
    FlsIf_ErasePage(CALIBRATION_CUR_A_ADDR);
    FlsIf_ProgramMultiple(CALIBRATION_CUR_A_ADDR,ADCM_t.cali.current.buff,8);
    /* 写入B区 */
    FlsIf_ErasePage(CALIBRATION_CUR_B_ADDR);
    FlsIf_ProgramMultiple(CALIBRATION_CUR_B_ADDR,ADCM_t.cali.current.buff,8);
    /* 计算CRC并存储: crc_buf[0]=电压CRC, crc_buf[1]=电流CRC */
    crc_buf[0] = 0xFFFFFFFF;
    FlsIf_ReadMultiple(CALIBRATION_CRC_ADDR, crc_buf, 8);
    crc_buf[1] = calibration_crc32(ADCM_t.cali.current.buff, 8);
    FlsIf_ErasePage(CALIBRATION_CRC_ADDR);
    FlsIf_ProgramMultiple(CALIBRATION_CRC_ADDR, crc_buf, 8);
    FLASH_Lock();
    FlsIf_cali_current_Init();
}

/**
* @brief 校准模式下，清除电压校准参数
*
*/
void calibration_cur_clear(calibration_Manager_t *calibration_Manager)
{
    FLASH_Unlock();
    FlsIf_ErasePage(CALIBRATION_CUR_A_ADDR);
    FlsIf_ErasePage(CALIBRATION_CUR_B_ADDR);
    FlsIf_ErasePage(CALIBRATION_CRC_ADDR);
    FLASH_Lock();
    FlsIf_cali_current_Init();
}

/**
* @brief 获取校准模式函数
*
*/
uint8_t get_calibration_mode_status(calibration_Manager_t *calibration_Manager) // 校准模式检查
{
    uint8_t tResult = 0;
    if(calibration_Manager->state == CALIBRATION_INTO)
    {
        tResult = 2;
    }
    else if(calibration_Manager->state == CALIBRATION_WAIT)
    {
        tResult = 1;
    }
    else
    {
        tResult = 0;
    }
    return tResult;
}

/**
* @brief 设置校准状态机退出校准模式状态
*
*/
static void Calibration_set_mode_exit(calibration_Manager_t *calibration_Manager) {
    calibration_Manager->state = CALIBRATION_EXIT;
}

/**
* @brief 校准模式命令行控制函数
*
*/
static void calibration(int argc, char**argv)
{
    int32_t value_k = 0;
    int32_t value_b = 0;
    if(memcmp(argv[1],"help",strlen("help")) == 0)
    {
        rt_kprintf("example: calibration $*module set $*value_k*1000 $*value_b*10\r\n");
        rt_kprintf("module: [vol,cur]\r\n");
        rt_kprintf("calibration vol set $*value_k*1000 $*value_b*10 ->calibration vol set value_k value_b \r\n");
        rt_kprintf("calibration cur set $*value_k*1000 $*value_b*10 ->calibration cur set value_k value_b \r\n");
    }
    if(memcmp(argv[1],"mode",strlen("mode")) == 0)
    {
        if(memcmp(argv[2],"enter",strlen("enter")) == 0)
        {
            // 状态机进入
            if(get_calibration_mode_status(&calibration_Manager) != 2)
            {
                chargingStateMachine_setcalibration(&machine);
            }
            else
            {
                rt_kprintf("--->Already in calibration mode...\r\n"); // 已处于校准模式
            }
        }
        else if(memcmp(argv[2],"exit",strlen("exit")) == 0)
        {
            if(get_calibration_mode_status(&calibration_Manager) == 2)
            {
                rt_kprintf("relay set off!!!\r\n");
                Relay_hold_off();
                rt_kprintf("wait to restart MCU!!!");
                Calibration_set_mode_exit(&calibration_Manager);
            }
        }
        else if(memcmp(argv[2],"get",strlen("get")) == 0)
        {
            if(get_calibration_mode_status(&calibration_Manager) == 2)
            {
                rt_kprintf("--->calibration mode...\r\n"); //
            }
            else if(get_calibration_mode_status(&calibration_Manager) == 1)
            {
                rt_kprintf("--->entering calibration mode--please wait...\r\n"); // 进入校准模式请等待
            }
            else
            {
                rt_kprintf("--->normal mode...\r\n"); // 普通模式
            }
        }
        else
        {
            rt_kprintf("Invalid command! Usage: calibration mode <enter/exit/get> ... ...\r\n");
        }
    }
    else
    {
        if(get_calibration_mode_status(&calibration_Manager) == 2)
        {
            if(memcmp(argv[1],"vol",strlen("vol")) == 0)
            {
                if(memcmp(argv[2],"clear",strlen("clear")) == 0)
                {
                    calibration_vol_clear(&calibration_Manager);
                }
                else if(memcmp(argv[2],"set",strlen("set")) == 0)
                {
                    value_k = atoi(argv[3]);
                    value_b = atoi(argv[4]);
                    calibration_vol_set(&calibration_Manager,value_k,value_b);
                }
            }
            else if(memcmp(argv[1],"cur",strlen("cur")) == 0)
            {
                if(memcmp(argv[2],"clear",strlen("clear")) == 0)
                {
                    calibration_cur_clear(&calibration_Manager);
                }
                else if(memcmp(argv[2],"set",strlen("set")) == 0)
                {
                    value_k = atoi(argv[3]);
                    value_b = atoi(argv[4]);
                    calibration_cur_set(&calibration_Manager,value_k,value_b);
                }
                else
                    rt_kprintf("Invalid command! Usage: calibration <vol/cur> <set/clear> ... ...\r\n");
            }
            else
                rt_kprintf("Invalid command! Usage: calibration <vol/cur> <set/clear> ... ...\r\n");
        }
        else
            rt_kprintf("Invalid command! Usage: calibration mode <enter/exit/get> ... ...\r\n");
    }
}
MSH_CMD_EXPORT(calibration , ...);

/**
* @brief 复位命令行控制函数
*
*/
static void restart(int argc, char**argv)
{
    rt_kprintf("relay set off!!!\r\n");
    Relay_hold_off();
    rt_kprintf("wait to restart MCU!!!");
    rt_thread_delay(1000);
    NVIC_SystemReset(); // NVIC复位
}
MSH_CMD_EXPORT(restart , duty);

/**
 * @brief 继电器命令行控制函数
 *
 */
static void relay(int argc, char**argv)
{
    if(get_calibration_mode_status(&calibration_Manager) == 2)
    {
        if(memcmp(argv[1],"help",strlen("help")) == 0)
        {
            rt_kprintf("interface: relay set $cmd\r\n");
            rt_kprintf("$cmd [on/off]\r\n");
        }
        if(memcmp(argv[1],"set",strlen("set")) == 0)
        {
            if(memcmp(argv[2],"on",strlen("on")) == 0)
            {
                Relay_hold_on();
                rt_kprintf("relay set on!!!\r\n");
            }
            else if(memcmp(argv[2],"off",strlen("off")) == 0)
            {
                Relay_hold_off();
                rt_kprintf("relay set off!!!\r\n");
            }
            else
            {
                rt_kprintf("Invalid command! Usage: relay set <on/off>\r\n");
            }
        }
    }
    else
    {
        rt_kprintf("Invalid command! Usage: relay set <on/off>\r\n");
    }
}
MSH_CMD_EXPORT(relay , duty);

/**
 * @brief 校准模式状态机更新
 *
 */
void Calibration_Update(calibration_Manager_t *calibration_Manager)
{
    calibration_Manager->timer += 5;
    switch (calibration_Manager->state)
    {
        case CALIBRATION_ENTRY: // 准备进入校准模式
            Relay_hold_off(); // 立即关闭继电器
            RTE_Call_DioIf_WriteChannel(HW_DIO_CH_OUT_SHRT,DIOIF_PIN_LOW); // 断开短路检测IO
            calibration_Manager->state = CALIBRATION_WAIT;
            calibration_Manager->timer = 0;
            rt_kprintf("-->entering calibration mode--please wait...\r\n"); // 进入校准模式请等待
            break;
        case CALIBRATION_WAIT: // 等待进入校准模式
            if (calibration_Manager->timer >= 200)
            {
                calibration_Manager->state = CALIBRATION_INTO;
                calibration_Manager->timer = 0;
                rt_kprintf("-->Successfully entered calibration mode...\r\n"); // 安全进入校准模式
            }
            break;
        case CALIBRATION_INTO: // 进入校准模式
            calibration_Manager->timer = 0;
            break;
        case CALIBRATION_EXIT: // 退出校准模式 退出即复位
            if (calibration_Manager->timer > 1000) {
                calibration_Manager->timer = 0;
                NVIC_SystemReset(); // NVIC复位
            }
            break;
        default:
            break;
    }
}
