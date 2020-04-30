#include "timer.h"
#include "interrupts.h"
#include "uart.h"
#include <os.h>

#define TIMER_BASE 0x20003000
#define TIMER_REG(off) (vu32*)(TIMER_BASE + off)

vu32* timerCS  = TIMER_REG(0x00);
vu32* timerCLO = TIMER_REG(0x04);
vu32* timerCHI = TIMER_REG(0x08);
vu32* timerC0  = TIMER_REG(0x0C);
vu32* timerC1  = TIMER_REG(0x10);
vu32* timerC2  = TIMER_REG(0x14);
vu32* timerC3  = TIMER_REG(0x18);

void sys_timer_irq_handler(int irq, void* data)
{
    // 0 and 2 are used by the GPU, so we don't handle or use those.
    u32 timedelta = (u32)data;

    __dmb();

    if (__builtin_expect(irq == 1, 1))
    {
        *timerCS = 0b0010;
        *timerC1 += timedelta;
    }
    else if (irq == 3)
    {
        *timerCS = 0b1000;
        *timerC3 += timedelta;
    }

    __dmb();

    OSTimeTick();
    OSIntExit();
    // if not exited, normal irq return
}

void timer_init(u32 timedelta)
{
    __dmb();

    int_disable_irq(IRQ_SYSTEM_TIMER_1);
    int_register_irq_handler(IRQ_SYSTEM_TIMER_1, sys_timer_irq_handler, (void*)timedelta);

    __dmb();

    u32 current_time = *timerCLO;
    *timerC1 = current_time + timedelta;
    *timerCS = 0b0010;

    __dmb();

    int_enable_irq(IRQ_SYSTEM_TIMER_1);

    __dmb();
}

void waitForMicro(u32 microseconds)
{
    volatile u32* timerReg = (volatile u32*)0x20003004;
    u32           startVal = *timerReg;

    while (*timerReg - startVal < microseconds)
        ;
}

