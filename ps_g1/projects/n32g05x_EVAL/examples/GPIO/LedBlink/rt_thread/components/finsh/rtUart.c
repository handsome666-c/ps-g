#include "rtuart.h"
rt_uint8_t uart_rx_buf[RT_UART_RX_BUF_LEN] = {0};
struct rt_semaphore shell_rx_sem; /* 定义一个静态信号量 */

void ls_rtUsart_config(void)
{
    /* 初始化串口接收 ringbuffer */
    rt_ringbuffer_init(&uart_rxcb, uart_rx_buf, RT_RX_BUF_SIZE);

    /* 初始化串口接收数据的信号量 */
    rt_sem_init(&(shell_rx_sem), "shell_rx", 0, 0);
}

