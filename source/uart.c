#include "gpio.h"
#include "timer.h"
#include "uart.h"

#define vu32 volatile uint32_t
#define vu8 volatile uint8_t

#define UART_BASE 0x20201000
#define UART_REG(off) (vu32*)(UART_BASE + off)

vu32* uart = UART_REG(0);
vu32* uart_lcrh = UART_REG(0x2C);
vu32* uart_cr = UART_REG(0x30);
vu32* uart_fr = UART_REG(0x18);
vu32* uart_ibrd = UART_REG(0x24);
vu32* uart_fbrd = UART_REG(0x28);
vu32* uart_imsc = UART_REG(0x38);
vu32* uart_icr = UART_REG(0x44);

void uart_init()
{
    static int initialized = 0;
    if (initialized != 0) return;
    // disable the UART while we set initialisation parameters
    __dmb();

    *uart_cr = 0;
    *uart_lcrh &= ~UART_LCRH_FIFO;

    __dmb();

    /* GPIO pin 14 (board pin 8) is TXD, 15 (10) is RXD.
     * Those are both the first alternative function of those pins. */
    GPIO_SetFunction(14, GPIO_FN_ALT0);
    GPIO_SetFunction(15, GPIO_FN_ALT0);

    __dmb();

    /* Set baud rate divisor to 1.6... (115200 b/s) */
    *uart_ibrd = 1;
    *uart_fbrd = 40;

    /* 8 bit mode, no parity, no break, enable fifos (buffering) */
    *uart_lcrh = (UART_LCRH_8BIT | UART_LCRH_FIFO | UART_LCRH_STP2);

    /* Disable all interrupts */
    *uart_icr = 0x7FF;
    *uart_imsc = 0;

    /* Enable UART transmission and reception using the Control Register */
    *uart_cr = (UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
    initialized++;
    __dmb();
}

char uart_getbyte()
{
    // Wait until the RX FIFO has bytes in it.
    while (*uart_fr & UART_FR_RXFE);

    return (*uart & 0xFF);
}

void uart_getline(char* buf)
{
    while ((*buf++ = uart_getbyte()) != '\n');
    *buf = 0;
}

void uart_sendbyte(char byte)
{
    // Wait until the TX FIFO has room.
    while (*uart_fr & UART_FR_TXFF);

    *uart = byte;
}

void uart_send(const char* text)
{
    char c;
    while ((c = *text++))
    {
        if (c == 0x0A)
            uart_sendbyte(0x0D);
        uart_sendbyte(c);
    }
}
