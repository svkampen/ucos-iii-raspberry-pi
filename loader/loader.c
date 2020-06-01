#include <uart.h>
#include <startup.h>
#include <printf.h>

void int_handle_irq() {};
uint32_t* OSTCBCurPtr;

void main()
{
    __dmb();

    uart_init();
    printf("Loader started, awaiting length...\n");

    uint32_t bytes_to_load = uart_getbyte();
    bytes_to_load += ((uint32_t)uart_getbyte()) << 8;
    bytes_to_load += ((uint32_t)uart_getbyte()) << 16;
    bytes_to_load += ((uint32_t)uart_getbyte()) << 24;

    uint32_t bytes_to_load_copy = bytes_to_load;

    uint32_t checksum = uart_getbyte();
    checksum += ((uint32_t)uart_getbyte()) << 8;
    checksum += ((uint32_t)uart_getbyte()) << 16;
    checksum += ((uint32_t)uart_getbyte()) << 24;


    uint8_t* offset = (uint8_t*)0x8000;

    while (bytes_to_load--)
    {
        *offset++ = uart_getbyte();
    }

    uint32_t recomputed_checksum = 0;
    while (bytes_to_load_copy--)
    {
        recomputed_checksum += *(--offset);
    }

    if (recomputed_checksum != checksum)
    {
        printf("Checksum failed: got %lu, expected %lu\n", recomputed_checksum, checksum);
        hang();
    }

    while (uart_getbyte() != 'l');

    __asm__("mov r0,#0x8000\nbx r0");
    __builtin_unreachable();
}
