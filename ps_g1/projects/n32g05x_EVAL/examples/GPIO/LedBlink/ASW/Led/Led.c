#include "Led.h"

Led_info_t Led_R;
Led_info_t Led_G;
Led_info_t Led_B;

Led_OpType Led_Op = LED_MODE_IDLE;
uint32_t Led_FaultCode = 0;

static uint8_t Led_Showloop_Flag = 0;
static uint8_t Led_Showloop_time_count = 0;

/*********************************************************************************************************
*函数名称  : void Led_Set_Mode(Led_info_t *led, uint8_t Mode, ...)
*参数      : void
*返回值    : void
*描述      : 灯语模块灯语设置
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
void Led_Set_Mode(Led_info_t *led, uint8_t Mode, ...)
{
    led->mode.mode = Mode;
    va_list args;
    va_start(args, Mode);
    switch (led->mode.mode) {
        // 占空比0%， ON
        case LED_MODE_KEEP_ON: led->mode.duty = 0; break;
        // 占空比100%， Off
        case LED_MODE_KEEP_OFF: led->mode.duty = 1000; break;
        // 闪烁模式，1s亮1s灭
        case LED_MODE_MODE_BLINK:
            led->mode.blink_interval = va_arg(args, int)/10;
            led->mode.blink_stop_cnt = va_arg(args, int);
            led->mode.blink_stop_time = va_arg(args, int)/10;
            break;
        // 呼吸模式(&led_x, mode, min_duty, max_duty, step, interval)
        case LED_MODE_MODE_BREATH:
            led->mode.breath_min_duty = va_arg(args, int);
            led->mode.breath_max_duty = va_arg(args, int);
            led->mode.breath_step = va_arg(args, int);
            led->mode.breath_interval = va_arg(args, int)/10;
            break;
        default:
            break;
    }
    va_end(args);
}

/*********************************************************************************************************
*函数名称  : void LedBeep_LedProcess(void)
*参数      : void
*返回值    : void
*描述      : 充电灯语模块故障灯语处理函数
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
static void LED_CHGMODE_Showfault(uint32_t fault_type)
{
    switch(fault_type)
    {
        case E_CHARGING_FAULT_LEAKAGE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 5, 3000);//间隔1s亮一次，亮5次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_CHARGING_FAULT_SHORT_CIRCUIT:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 7, 3000);//间隔1s亮一次，亮7次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_CHARGING_FAULT_RELAY_STICK:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 8, 3000);//间隔1s亮一次，亮8次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_CHARGING_FAULT_OVER_CURRENT:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 4, 3000);//间隔1s亮一次，亮4次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_CHARGING_FAULT_OVER_UNDER_VOLTAGE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 3, 3000);//间隔1s亮一次，亮3次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_CHARGING_FAULT_OVER_MCUTEMPERATURE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 6, 3000);//间隔1s亮一次，亮6次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_CHARGING_FAULT_CP_ABNORMAL:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 2, 3000);//间隔1s亮一次，亮2次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        // case E_CHARGING_FAULT_DIODE_CHECK://二极管故障5s一个亮灭循环，每10ms，duty±40
        //     Led_Set_Mode(&Led_R,LED_MODE_KEEP_OFF);
        //     Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
        //     Led_Set_Mode(&Led_G,LED_MODE_MODE_BREATH, 0, 1000, 40, 1);//min, max, step, interval
        //     Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
        //     break;
        case E_CHARGING_FAULT_GROUND_FAULT:
            Led_Set_Mode(&Led_R,LED_MODE_KEEP_ON);//红常亮
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        default:
            break;
    }
}

/*********************************************************************************************************
*函数名称  : void LedBeep_LedProcess(void)
*参数      : void
*返回值    : void
*描述      : 放电灯语模块故障灯语处理函数
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
static void LED_DISCHGMODE_Showfault(uint32_t fault_type)
{
    switch(fault_type)
    {
        case E_DISCHARGING_FAULT_LEAKAGE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 4, 3000);//间隔1s亮一次，亮4次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_DISCHARGING_FAULT_RELAY_STICK:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 6, 3000);//间隔1s亮一次，亮6次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_DISCHARGING_FAULT_OVER_CURRENT:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 8, 3000);//间隔1s亮一次，亮8次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_DISCHARGING_FAULT_OVER_UNDER_VOLTAGE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 3, 3000);//间隔1s亮一次，亮3次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_DISCHARGING_FAULT_OVER_MCUTEMPERATURE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 5, 3000);//间隔1s亮一次，亮5次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        case E_DISCHARGING_FAULT_OVER_NTCTEMPERATURE:
            Led_Set_Mode(&Led_R,LED_MODE_MODE_BLINK, 1000, 7, 3000);//间隔1s亮一次，亮7次，等待3s循环
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            break;
        default:
            break;
    }
}

/*********************************************************************************************************
*函数名称  : static void APP_PWM_LED(Led_info_t *led, uint16_t Duty)
*参数      : Led_info_t *led, uint16_t Duty
*返回值    : void
*描述      : 灯语模块PWM控制
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
static void APP_PWM_LED(Led_info_t *led, uint16_t Duty)
{
    switch(led->led_x)
    {
        case LED_LEDG:
            RTE_Call_PwmIf_Set_Duty(TIM4, OC1, Duty);
            break;
        case LED_LEDB:
            RTE_Call_PwmIf_Set_Duty(TIM3, OC2, Duty);
            break;
        case LED_LEDR:
            RTE_Call_PwmIf_Set_Duty(TIM3, OC1, Duty);
            break;
        default:
            break;
    }
}

/*********************************************************************************************************
*函数名称  : void led_auto(Led_info_t *led)
*参数      : void
*返回值    : void
*描述      : 灯语模块执行函数
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
void led_auto(Led_info_t *led)
{
    switch (led->mode.mode)
    {
        // 占空比0%， ON
        case LED_MODE_KEEP_ON:
            APP_PWM_LED(led, led->mode.duty);
            led->mode.time_10ms_cnt = 0;
            led->mode.blink_cnt = 0; //闪烁次数清零
            led->mode.blink_work_state = LED_BLINKING;
            led->mode.breath_current_duty = 0; //呼吸灯占空比清零
            led->mode.breath_dir = BREATH_INCREASE;
            break;

        // 占空比100%， Off
        case LED_MODE_KEEP_OFF:
            APP_PWM_LED(led, led->mode.duty);
            led->mode.time_10ms_cnt = 0;
            led->mode.blink_cnt = 0; //闪烁次数清零
            led->mode.blink_work_state = LED_BLINKING;
            led->mode.breath_current_duty = 0; //呼吸灯占空比清零
            led->mode.breath_dir = BREATH_INCREASE;
            break;

        // 闪烁模式，1s亮1s灭(&led_x, mode, interval)
        case LED_MODE_MODE_BLINK:
            led->mode.time_10ms_cnt += 1; //每次+1为10ms
            if(led->mode.blink_work_state == LED_BLINKING) //闪烁模式
            {
                if(led->mode.time_10ms_cnt < led->mode.blink_interval) // 检查是否到达更新时间
                {
                    return;
                }
                else
                {
                    led->mode.blink_current_level = !led->mode.blink_current_level;
                }
                if(led->mode.blink_current_level == BLINK_OFF)
                {
                    APP_PWM_LED(led, 1000);
                    led->mode.time_10ms_cnt = 0;
                    if(led->mode.blink_cnt >= led->mode.blink_stop_cnt) //超过最大闪烁次数
                    {
                        led->mode.blink_work_state = LED_PAUSING;
                        led->mode.blink_cnt = 0;
                    }
                }
                else
                {
                    APP_PWM_LED(led, 0);
                    if(led->mode.blink_stop_cnt != 0xFFFF)
                    {
                        led->mode.blink_cnt += 1; //闪烁次数+1
                    }
                    led->mode.time_10ms_cnt = 0;
                }
            }
            else if(led->mode.blink_work_state == LED_PAUSING) //暂停模式
            {
                // 检查是否到达更新时间
                if(led->mode.time_10ms_cnt < led->mode.blink_stop_time) //最小等待时间
                {
                    return;
                }
                else{
                    led->mode.blink_work_state = LED_BLINKING;
                }
            }
            else{

            }
            led->mode.breath_current_duty = 0; //呼吸灯占空比清零
            led->mode.breath_dir = BREATH_INCREASE;
            break;

        // 呼吸模式(&led_x, mode, min_duty, max_duty, current_duty, step, interval)
        case LED_MODE_MODE_BREATH:
            led->mode.time_10ms_cnt += 1; //每次+1为10ms
            if (led->mode.time_10ms_cnt < led->mode.breath_interval) { // 检查是否到达更新时间
                return;
            }
            if(led->mode.breath_dir == BREATH_INCREASE)
            {
                led->mode.breath_current_duty += led->mode.breath_step;
                APP_PWM_LED(led, led->mode.breath_current_duty);
                led->mode.time_10ms_cnt = 0;
                // 达到最大亮度，切换为递减状态
                if (led->mode.breath_current_duty >= led->mode.breath_max_duty) {
                    led->mode.breath_current_duty = led->mode.breath_max_duty;
                    led->mode.breath_dir = BREATH_DECREASE;
                    led->mode.time_10ms_cnt = 0;
                }
            }
            else if(led->mode.breath_dir == BREATH_DECREASE)
            {
                led->mode.breath_current_duty -= led->mode.breath_step;
                APP_PWM_LED(led, led->mode.breath_current_duty);
                led->mode.time_10ms_cnt = 0;
                // 达到最小亮度，切换为递增状态
                if (led->mode.breath_current_duty <= led->mode.breath_min_duty) {
                    led->mode.breath_current_duty = led->mode.breath_min_duty;
                    led->mode.breath_dir = BREATH_INCREASE;
                    led->mode.time_10ms_cnt = 0;
                }
            }
            else{

            }
            led->mode.blink_cnt = 0; //闪烁次数清零
            led->mode.blink_work_state = LED_BLINKING;
            break;
        default: break;
    }
}

/*********************************************************************************************************
*函数名称  : void Led_ShowLoop(void)
*参数      : void
*返回值    : void
*描述      : 灯语模块开机自检灯语
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
static void Led_ShowLoop(void)
{
    Led_Showloop_time_count += 1;
    if(Led_Showloop_time_count >= 100)
    {
        if(Led_Showloop_Flag == 0)//蓝
        {
            Led_Set_Mode(&Led_R,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_ON);
            Led_Showloop_time_count = 0;
            Led_Showloop_Flag = 0x01;
        }
        else if(Led_Showloop_Flag == 0x01)//绿
        {
            Led_Set_Mode(&Led_R,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_ON);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            Led_Showloop_time_count = 0;
            Led_Showloop_Flag = 0x02;
        }
        else if(Led_Showloop_Flag == 0x02)//蓝
        {
            Led_Set_Mode(&Led_R,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_ON);
            Led_Showloop_time_count = 0;
            Led_Showloop_Flag = 0x03;
        }
        else if(Led_Showloop_Flag == 0x03)//红
        {
            Led_Set_Mode(&Led_R,LED_MODE_KEEP_ON);
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            Led_Showloop_time_count = 0;
            Led_Showloop_Flag = 0x04;
        }
        else if(Led_Showloop_Flag == 0x04)//灭
        {
            Led_Set_Mode(&Led_R,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_G,LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B,LED_MODE_KEEP_OFF);
            Led_Showloop_time_count = 0;
            Led_Showloop_Flag = 0x00;
        }
    }
}

/*********************************************************************************************************
*函数名称  : void LedBeep_LedProcess(void)
*参数      : void
*返回值    : void
*描述      : 灯语模块灯语处理函数
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
static void Led_Process(void)
{
    if(get_calibration_mode_status(&calibration_Manager) == 2)
    {
        Led_Set_Mode(&Led_B,LED_MODE_KEEP_ON);
        Led_Set_Mode(&Led_R,LED_MODE_KEEP_ON);
        Led_Set_Mode(&Led_G,LED_MODE_KEEP_ON);
        return;
    }

if(getSysState() == SYSSTATE_CHARGING)
{
    Led_Op = (Led_OpType)APP_CHG_GetStatusLED(); /*todo:获取充电状态*/
    switch(Led_Op)
    {
        case LED_MODE_POST: //开机自检，蓝-绿-蓝-红-灭 顺序闪烁（1s）
            Led_ShowLoop();
            break;
        case LED_MODE_IDLE: //空闲，蓝常亮
            Led_Set_Mode(&Led_B, LED_MODE_KEEP_ON);
            Led_Set_Mode(&Led_R, LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_G, LED_MODE_KEEP_OFF);
            break;
        case LED_MODE_CONN: //插枪，连接，绿闪烁
            Led_Set_Mode(&Led_G, LED_MODE_MODE_BLINK, 1000, 0xFFFF); //间隔1s闪烁1次，0xFFFF循环
            Led_Set_Mode(&Led_R, LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
            break;
        case LED_MODE_CHARGING: //充电，绿色呼吸，二极管正常2s一个亮灭循环，每10ms，duty+100
            if(DIODE_RESULT_NORMAL == APP_CHG_GetDiodeStatus())
            {
                Led_Set_Mode(&Led_R, LED_MODE_KEEP_OFF);
                Led_Set_Mode(&Led_G, LED_MODE_MODE_BREATH, 0, 1000, 10, 10); //min, max, step, interval
                Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
            }
            else
            {
                Led_Set_Mode(&Led_R, LED_MODE_KEEP_OFF);
                Led_Set_Mode(&Led_G, LED_MODE_MODE_BREATH, 0, 1000, 10, 25); //min, max, step, interval
                Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
            }
            break;
        case LED_MODE_VPAUSE: //暂停，绿常亮
            Led_Set_Mode(&Led_R, LED_MODE_KEEP_OFF);
            Led_Set_Mode(&Led_G, LED_MODE_KEEP_ON);
            Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
            break;
        case LED_MODE_FAULT: //故障，红亮n次停3s循环
            Led_FaultCode = APP_CHG_GetFault(); /*todo:获取故障状态*/
            LED_CHGMODE_Showfault(Led_FaultCode);
            break;
        default:
            break;
    }
}
else if(getSysState() == SYSSTATE_DISCHARGING)
{
    Led_Op = (Led_OpType)APP_DISCHG_GetStatusLED(); /*todo:获取放电状态*/
    switch(Led_Op)
    {
        case LED_MODE_POST: //开机自检，蓝-绿-蓝-红-灭 顺序闪烁（1s）
            Led_ShowLoop();
            break;
        case LED_MODE_DISCHARGING_WAIT: //放电等待，黄常亮
            Led_Set_Mode(&Led_R, LED_MODE_KEEP_ON);
            Led_Set_Mode(&Led_G, LED_MODE_KEEP_ON);
            Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
            break;
        case LED_MODE_DISCHARGING: //放电中，黄呼吸
            Led_Set_Mode(&Led_R, LED_MODE_MODE_BREATH, 0, 1000, 10, 10);
            Led_Set_Mode(&Led_G, LED_MODE_MODE_BREATH, 0, 1000, 10, 10); //min, max, step, interval
            Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
            break;
        case LED_MODE_DISCHARGING_FAULT: //故障，红亮n次停3s循环
            Led_FaultCode = APP_DISCHG_GetFault(); /*todo:获取故障状态*/
            LED_DISCHGMODE_Showfault(Led_FaultCode);
            break;
        default:
            break;
    }
}
else
{

}

}
/*********************************************************************************************************
*函数名称  : void Led_MainFunction(void)
*参数      : void
*返回值    : void
*描述      : 灯语模块主函数
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
void Led_MainFunction(void)
{
    Led_Process(); //10ms
    led_auto(&Led_R);
    led_auto(&Led_G);
    led_auto(&Led_B);
}

/*********************************************************************************************************
*函数名称  : void APP_Led_Init(void)
*参数      : void
*返回值    : void
*描述      : 灯语模块初始化
*编辑时间  :
*备注      : 初版
*********************************************************************************************************/
void APP_Led_Init(void)
{
    Led_R.led_x = LED_LEDR;
    Led_G.led_x = LED_LEDG;
    Led_B.led_x = LED_LEDB;
    Led_Set_Mode(&Led_R, LED_MODE_KEEP_OFF);
    Led_Set_Mode(&Led_G, LED_MODE_KEEP_OFF);
    Led_Set_Mode(&Led_B, LED_MODE_KEEP_OFF);
}
