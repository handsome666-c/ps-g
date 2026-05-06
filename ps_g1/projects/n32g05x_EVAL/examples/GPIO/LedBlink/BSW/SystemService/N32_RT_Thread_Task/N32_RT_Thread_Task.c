#include "n32g05x_rcc.h"
#include "rtthread.h"
#include "n32g05x_uart.h"
#include "log.h"
#include "N32_RT_Thread_Task.h"
#include "WdgM.h"
#include "EcuM.h"
#include "Pwmif.h"
#include "AdcM.h"
#include "AdcIf.h"
#include "DioIf.h"
#include "Timrif.h"
#include "FlashIf.h"
#include "UartIf.h"
#include "cp.h"
#include "charging.h"
#include "Selfcheck.h"
#include "Diodecheck.h"
#include "RTE_BSW.h"
#include "RTE_ASW.h"
#include "string.h"

/* USER CODE BEGIN PV */

/*定义线程控制块CORE*/
static struct rt_thread core_thread;
/* 定义线程栈时要求 RT_ALIGN_SIZE 个字节对齐 */
ALIGN(RT_ALIGN_SIZE)
/* 定义线程栈 */
static rt_uint8_t rt_core_thread_stack[1024];

/*定义线程控制块Task_1ms*/
static struct rt_thread Task_1ms_thread;
/* 定义线程栈时要求 RT_ALIGN_SIZE 个字节对齐 */
ALIGN(RT_ALIGN_SIZE)
/* 定义线程栈 */
static rt_uint8_t rt_Task_1ms_thread_stack[1024];

/*定义线程控制块Task_5ms*/
static struct rt_thread Task_5ms_thread;
/* 定义线程栈时要求 RT_ALIGN_SIZE 个字节对齐 */
ALIGN(RT_ALIGN_SIZE)
/* 定义线程栈 */
static rt_uint8_t rt_Task_5ms_thread_stack[1024];

/*定义线程控制块Task_10ms*/
static struct rt_thread Task_10ms_thread;
/* 定义线程栈时要求 RT_ALIGN_SIZE 个字节对齐 */
ALIGN(RT_ALIGN_SIZE)
/* 定义线程栈 */
static rt_uint8_t rt_Task_10ms_thread_stack[1024];

/*定义线程控制块Task_1s*/
static struct rt_thread Task_1s_thread;
/* 定义线程栈时要求 RT_ALIGN_SIZE 个字节对齐 */
ALIGN(RT_ALIGN_SIZE)
/* 定义线程栈 */
static rt_uint8_t rt_Task_1s_thread_stack[512];

//声明线程函数
static void core_thread_entry(void* parameter);
static void Task_1ms_thread_entry(void* parameter);
static void Task_5ms_thread_entry(void* parameter);
static void Task_10ms_thread_entry(void* parameter);
static void Task_1s_thread_entry(void* parameter);

/**********************************************************************
*函数名称  : void EcuM_Init(void)
*参数      : void
*返回值    : void
*描述      : 初始化
*编辑时间  :
初版
*备注      :
***********************************************************************/
void EcuM_InitRT_Thread(void)
{
    WdgM_Trigger();

    rt_err_t result;
    //初始化线程
    rt_thread_init(&core_thread,         /*线程控制块*/
                   "core_thread",        /*线程名字*/
                   core_thread_entry,     /*线程入口函数*/
                   RT_NULL,               /*线程入口函数参数*/
                   &rt_core_thread_stack[0], /*线程栈起始地址*/
                   sizeof(rt_core_thread_stack), /*线程栈大小*/
                   3,                     /*线程的优先级*/
                   20);                   /*线程时间片*/

    //启动线程
    rt_thread_startup(&core_thread);

    /* init Task_1ms_thread*/
    result = rt_thread_init(&Task_1ms_thread, "Task_1ms", Task_1ms_thread_entry, RT_NULL, (rt_uint8_t *)&rt_Task_1ms_thread_stack[0], sizeof(rt_Task_1ms_thread_stack), 4, 5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&Task_1ms_thread);
    }

    /* init Task_5ms_thread*/
    result = rt_thread_init(&Task_5ms_thread, "Task_5ms", Task_5ms_thread_entry, RT_NULL, (rt_uint8_t *)&rt_Task_5ms_thread_stack[0], sizeof(rt_Task_5ms_thread_stack), 4, 5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&Task_5ms_thread);
    }

    /* init Task_10ms_thread*/
    result = rt_thread_init(&Task_10ms_thread, "Task_10ms", Task_10ms_thread_entry, RT_NULL, (rt_uint8_t *)&rt_Task_10ms_thread_stack[0], sizeof(rt_Task_10ms_thread_stack), 4, 5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&Task_10ms_thread);
    }

    /* init Task_1s_thread*/
    result = rt_thread_init(&Task_1s_thread, "Task_1s", Task_1s_thread_entry, RT_NULL, (rt_uint8_t *)&rt_Task_1s_thread_stack[0], sizeof(rt_Task_1s_thread_stack), 4, 5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&Task_1s_thread);
    }

    while (1)
    {
        rt_thread_delay(1000); /* 延时 1000 个 tick */
    }
}

//定义线程函数core_thread
static void core_thread_entry(void* parameter)
{
    // rt_kprintf(">>> core thread start!\n");
    while (1)
    {
        WdgM_Trigger();
        rt_thread_delay(50); /* 延时 50 个 tick */
    }
}

//定义线程函数Task_1ms
static void Task_1ms_thread_entry(void* parameter)
{
    while(1)
    {
        RTE_Call_ASW_MainFunction_1ms();
        AdcM_CallbackSampleValue(); // ADC采样数据计算
        rt_thread_delay(1);
    }
}

//定义线程函数Task_5ms
static void Task_5ms_thread_entry(void* parameter)
{
    while(1)
    {
        RTE_Call_ASW_MainFunction_5ms();
        rt_thread_delay(5);
    }
}

//定义线程函数Task_10ms
static void Task_10ms_thread_entry(void* parameter)
{
    while(1)
    {
        RTE_Call_ASW_MainFunction_10ms();
        rt_thread_delay(10);
    }
}

//定义线程函数Task_1s
static void Task_1s_thread_entry(void* parameter)
{
    while(1)
    {
        RTE_Call_ASW_MainFunction_1s();
        rt_thread_delay(1000);
    }
}
