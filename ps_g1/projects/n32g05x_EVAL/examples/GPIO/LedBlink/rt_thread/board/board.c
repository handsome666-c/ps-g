/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include "n32g05x_uart.h"
#include "log.h"
#include "rtuart.h"
#include "ringbuf.h"
#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    log_init();
    
    /* System Clock Update */
    SystemCoreClockUpdate();
    
    /* System Tick Configuration */
    _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
	
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

void UART1_IRQHandler(void)
{
    /*等待串口1输入数据*/
    if( UART_GetFlagStatus( UART1, UART_FLAG_RXDNE ) != RESET )
    {
        rt_interrupt_enter();      // 进入中断，通知RT-Thread内核
        /* 读取到数据，将数据存入 ringbuffer */
        rt_ringbuffer_putchar(&uart_rxcb, (rt_uint8_t)(UART_ReceiveData( UART1 )));
        rt_sem_release(&shell_rx_sem);  // 释放信号量，通知有数据可读
        rt_interrupt_leave();       // 退出中断
    }
}

void rt_hw_console_output(const char *str)
{
    /* empty console output */
    rt_enter_critical();  // 进入临界区，防止多线程打断输出

    while (*str != '\0')
    {
        if (*str == '\n')  // 遇到换行符，自动补上\r（回车），适配终端
        {
            while (UART_GetFlagStatus(UART1, UART_FLAG_TXDE) == RESET);
            UART_SendData(UART1, '\r');
        }
        while (UART_GetFlagStatus(UART1, UART_FLAG_TXDE) == RESET);
        UART_SendData(UART1, *str++);
    }

    rt_exit_critical();  // 退出临界区
}


//使用Finsh组件三步骤：
//1.实现该函数及rt_hw_console_output函数；
//2.rtconfig.h中开启RT_USING_FINSH宏;
//3.添加Finsh组件(cmd.c、msh.c、shell.c);
char rt_hw_console_getchar(void)
{
    char ch = 0;
    /* 从 ringbuffer 中拿出数据 */
    while (rt_ringbuffer_getchar(&uart_rxcb, (rt_uint8_t *)&ch) != 1)
    {
        rt_sem_take(&shell_rx_sem, RT_WAITING_FOREVER);
    }
    return ch;
}






