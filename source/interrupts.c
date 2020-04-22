#include "interrupts.h"
#include "uart.h"
#include "io.h"

interrupt_handler_t irq_handlers[BCM2835_IRQ_MAX];

vu32* irq_basic_pend    = IRQ_REG(0x200);
vu32* irq_pend_1        = IRQ_REG(0x204);
vu32* irq_pend_2        = IRQ_REG(0x208);
vu32* irq_enable_1      = IRQ_REG(0x210);
vu32* irq_enable_2      = IRQ_REG(0x214);
vu32* irq_basic_enable  = IRQ_REG(0x218);
vu32* irq_disable_1     = IRQ_REG(0x21C);
vu32* irq_disable_2     = IRQ_REG(0x220);
vu32* irq_basic_disable = IRQ_REG(0x224);

void int_handle_irq()
{
    // Check if this is a timer interrupt
    u32 pending_status        = *irq_basic_pend;
    u32 further_banks_pending = pending_status & 0x300;
    u32 irqBaseNum;
    u32 first_set;

#ifdef DEBUGINTR
    uart_send("Pending: ");
    printi(pending_status);
#endif

    /* If there's a non-GPU interrupt: */
    if (pending_status & ~0xFFFFF300)
    {
#ifdef DEBUGINTR
        uart_send("Non-GPU interrupt: ");
        printi(pending_status);
#endif
        irqBaseNum = 64;
        goto emit_interrupt;
    }

    if (further_banks_pending & 0x100)
    {
        // One of the first 32 GPU interrupts is pending
        pending_status = *irq_pend_1;
#ifdef DEBUGINTR
        uart_send("GPU 1 interrupt: ");
        printi(pending_status);
#endif
        irqBaseNum     = 0;
        if (pending_status) // always the case? copied from Real-Pi
            goto emit_interrupt;
    }

    if (further_banks_pending & 0x200)
    {
        // One of the last 32 GPU interrupts is pending
        pending_status = *irq_pend_2;
#ifdef DEBUGINTR
        uart_send("GPU 2 interrupt: ");
        printi(pending_status);
#endif
        irqBaseNum     = 32;
        if (pending_status) goto emit_interrupt;
    }

    return;

emit_interrupt:
    first_set = __builtin_ffs(pending_status) - 1;

    interrupt_handler_t hdlr = irq_handlers[irqBaseNum + first_set];

    if (hdlr.interrupt_handler_fn)
    {
        hdlr.interrupt_handler_fn(irqBaseNum + first_set, hdlr.data);
    }
    else
    {
        /* Nothing handles this interrupt, so disable it. */
        int_disable_irq(irqBaseNum + first_set);
    }
}

bool int_enable_irq(int irq)
{
    int bank = irq / 32;
    int off  = irq % 32;

    if (bank > 2 || (bank == 2 && off > 7)) return false;

    /* Since enable_1, enable_2 and ARM enable are
     * sequential, we can index irq_enable_1 with
     * the bank number. */
    irq_enable_1[bank] |= (1 << off);
    return true;
}

bool int_disable_irq(int irq)
{
    int bank = irq / 32;
    int off  = irq % 32;

    if (bank > 2 || (bank == 2 && off > 7)) return false;

    /* Since disable_1, disable_2 and ARM disable are
     * sequential, we can index irq_disable_1 with
     * the bank number. */
    irq_disable_1[bank] |= (1 << off);
    return true;
}

void int_register_irq_handler(int irq, void (*fn)(int irq, void* data), void* data)
{
    irq_handlers[irq] = (interrupt_handler_t){ fn, data };
    int_enable_irq(irq);
}

void int_unregister_irq_handler(int irq)
{
    int_disable_irq(irq);
    irq_handlers[irq] = (interrupt_handler_t){ 0, 0 };
}

void int_init()
{
    for (int i = 0; i < BCM2835_IRQ_MAX; ++i)
    {
        irq_handlers[i] = (interrupt_handler_t){ 0, 0 };
    }

    __dmb();

    *irq_disable_1 = 0xFFFFFFFF;
    *irq_disable_2 = 0xFFFFFFFF;
    *irq_basic_disable = 0b11111111;

    __dmb();
}
