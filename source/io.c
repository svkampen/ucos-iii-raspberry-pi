#include "uart.h"
void printi(u32 val)
{
    /* 32-bit number is max 10 digits, 1 extra for null byte. */
    char buf[11];
    int idx = 0;
    while (val > 9)
    {
        buf[idx++] = (val % 10) + '0';
        val /= 10;
    }

    uart_sendbyte((val % 10) + '0');

    /* Reverse output. */
    while (idx-- >= 0)
    {
        uart_sendbyte(buf[idx]);
    }

    uart_sendbyte('\r');
    uart_sendbyte('\n');
}

void printh(u32 val)
{
    char buf[9];
    int idx = 0;

    while (val > 15)
    {
        buf[idx++] = "0123456789ABCDEF"[val % 16];
        val /= 16;
    }

    buf[idx++] = "0123456789ABCDEF"[val % 16];

    while (idx < 9)
    {
        buf[idx++] = '0';
    }

    uart_sendbyte('0');
    uart_sendbyte('x');

    while (idx-- >= 0)
    {
        uart_sendbyte(buf[idx]);
    }

    uart_sendbyte('\r');
    uart_sendbyte('\n');
}
