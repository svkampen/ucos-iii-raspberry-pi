#include "atag.h"
#include "interrupts.h"
#include "gpio.h"
#include "io.h"
#include "timer.h"
#include "uart.h"
#include "includes.h"

extern u32 __kernel_top;
u32        __memory_top;

#define MILLISEC 1000
#define SEC (1000*MILLISEC)

void main(u32 zero, u32 machine_id, const struct ATag* atag_location)
{
    setGpioFunction(16, 1, GPIO_PUD_OFF);

    uart_init();

    setGpio(16, 0);

    uart_send("UART initialized. You should be able to see this.\n");

    uart_send("Reading ATAGs...\n");

    arch_info_init((const struct ATag*)0x100);

    uart_send("Setting up the system timer interrupt...\n");

    initialize_interrupts();
    timer_enable_tick(10*MILLISEC);

    uart_send("Set up? Fingers crossed...\n");
}
