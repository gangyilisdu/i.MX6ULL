#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "imx6ul.h"

void uart_init(void);
void uart_io_init(void);
void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz);
void uart_disable(UART_Type *base);
void uart_enable(UART_Type *base);
void uart_softreset(UART_Type *base);
void raise(int sig_nr);


#endif