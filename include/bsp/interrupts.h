#include "types.h"
#include <stdbool.h>
#define BCM2835_IRQ_MAX (64 + 8)

#define IRQ_BASE 0x2000B000
#define IRQ_REG(off) (vu32*)(IRQ_BASE + off)

#define IRQ_SYSTEM_TIMER_1 1
#define IRQ_SYSTEM_TIMER_3 3
#define IRQ_UART 57

typedef struct
{
    void (*interrupt_handler_fn)(int irq, void* data);
    void* data;
} interrupt_handler_t;

extern void int_enable_irqs();

bool int_enable_irq(int irq);
void int_handle_irq();
bool int_disable_irq(int irq);
void int_register_irq_handler(int irq, void (*fn)(int irq, void* data), void* data);
void int_unregister_irq_handler(int irq);
void int_init();
