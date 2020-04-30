#pragma once
#include "types.h"
#define UART_CR_UARTEN 1 << 0
#define UART_CR_TXE (1 << 8)
#define UART_CR_RXE (1 << 9)
#define UART_CR_RTSEN (1 << 14)
#define UART_CR_CTSEN (1 << 15)
#define UART_LCRH_8BIT (3 << 5)
#define UART_LCRH_FIFO (1 << 4)
#define UART_LCRH_STP2 (1 << 3)
#define UART_FR_TXFE (1 << 7)
#define UART_FR_RXFF (1 << 6)
#define UART_FR_TXFF (1 << 5)
#define UART_FR_RXFE (1 << 4)
#define UART_FR_BUSY (1 << 3)

void uart_init();
char uart_getbyte();
void uart_getline(char* buf);
void uart_sendbyte(char byte);
void _putchar(char byte);
void uart_send(const char* text);
