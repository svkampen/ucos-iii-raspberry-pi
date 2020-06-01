#include "gpio.h"

#define GPIO_BASE 0x20200000
#define GPIO_ADDR(off) (vu32*)(GPIO_BASE + off)

volatile u32* gpioAddr = GPIO_ADDR(0);
volatile u32* gpioPud  = GPIO_ADDR(0x94);
volatile u32* gpioPudclk0 = GPIO_ADDR(0x98);

void GPIO_SetFunction(u32 pin, u32 function)
{
   if (pin > 53 || function > 7)
        return;

    // *gpioPud = pud;
    // wait_cycles(150);
    // *(gpioPudclk0 + (pin / 32)) = (1 << (pin % 32));
    // wait_cycles(150);

    // *gpioPud = GPIO_PUD_OFF;
    // *(gpioPudclk0 + (pin / 32)) = 0;

    volatile u32* gpioReg = gpioAddr;

    while (pin > 9)
    {
        pin -= 10;
        gpioReg++;
    }

    pin *= 3;
    function <<= pin;
    *gpioReg &= ~(7 << pin);
    *gpioReg |= function;
}

void GPIO_Set(u32 pin, u32 value)
{
    if (pin > 53) return;

    volatile u32* gpioReg = gpioAddr;

    u32 pinBank = (pin >> 5);

    gpioReg += pinBank;

    pin &= 31;
    if (value == 0)
        *(gpioReg + 10) = (1 << pin);
    else
        *(gpioReg + 7) = (1 << pin);
}
