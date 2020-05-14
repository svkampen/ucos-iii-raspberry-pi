#include <uart.h>
#include <printf.h>

void int_handle_irq() {};
uint32_t* OSTCBCurPtr;

void main(void)
{
    __dmb();

    uart_init();

    printf("Loader started, awaiting length...\n");

    uint32_t bytes_to_load = uart_getbyte();
    bytes_to_load += ((uint32_t)uart_getbyte()) << 8;
    bytes_to_load += ((uint32_t)uart_getbyte()) << 16;
    bytes_to_load += ((uint32_t)uart_getbyte()) << 24;

    uint8_t* offset = (uint8_t*)0x8000;
    while (bytes_to_load--)
    {
        *offset++ = uart_getbyte();
    }

    while (uart_getbyte() != 'l');

    goto *((void*)0x8000); // (gcc extension -> go to 0x8000)
}
