#ifndef __RTUART_H
#define __RTUART_H

#include "stdint.h"
#include "rtthread.h"

#include "rtdef.h"
#include "rtthread.h"
#include "ringbuf.h"

//#include "segger_rtt.h"

//#include "uartconfig.h"
#include "stdint.h"
//#include "FM33Lc0xx.h"
//#include "segger_rtt.h"
#include "string.h"
//#include "dev.h"


#define RT_TX_BUF_SIZE 256
#define RT_RX_BUF_SIZE 256
#define RT_UART_RX_BUF_LEN 256


#define rt_ringbuffer_space_len(rb) ((rb)->buffer_size - rt_ringbuffer_data_len(rb))
extern void ls_rtUsart_config(void);
extern struct rt_semaphore shell_rx_sem; /* 定义一个静态信号量 */

extern void rt_hw_console_output(const char *str);

#endif

