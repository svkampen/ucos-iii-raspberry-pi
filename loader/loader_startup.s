.extern __bss_start
.extern __bss_end
.extern __init_end
.extern __data_end
.extern main
.include "macros.inc"

.section .init
.global _start


/* The ARM exception vectors do not consist of addresses of exception handlers,
 * but rather the actual branch instructions. Seeing as our kernel is started
 * at 0x8000 by the boot code, we need to copy these over, which we will do
 * shortly, in the reset handler. Conveniently, that's the first EV,
 * so we'll branch to it immediately.
 */

_start:
    ldr pc, reset_handler
    ldr pc, undefined_handler
    ldr pc, swi_handler
    ldr pc, prefetch_handler
    ldr pc, data_handler
    ldr pc, unused_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler

reset_handler:     .word reset
undefined_handler: .word hang
swi_handler:       .word swi
prefetch_handler:  .word hang
data_handler:      .word hang
unused_handler:    .word hang
irq_handler:       .word irq
fiq_handler:       .word hang

reset:
    mov r0,#0x8000
    mov r1,#0

    /* Move 8*4*2 = 64 bytes from 0x8000 (_start) to the exception vectors */
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}

    /* Set up stack pointers for different modes */
    // Go into processor mode IRQ (but with IRQs and FIQs disabled)
    // PSR_IRQ_MODE | PSR_FIQ_DIS | PSR_IRQ_DIS
    // We'll set the stack for IRQ mode at 0x8000 (growing downward)
    mov r0,#0xD2
    msr cpsr_c,r0
    mov sp,#0x8000

    // SVC mode: stack at 0x8000000 (128 MiB)
    mov r0,#0xD3
    msr cpsr_c,r0
    mov sp,#0x8000000

// Zero out C BSS

    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0

zero_loop$:
    cmp r0,r1
    strls r2,[r0],#4
    bls zero_loop$

// Copy serial loader high up in memory (to 100MB)
    ldr r0,=__init_end
    ldr r1,=0x6400000
    ldr r2,=__data_end

copy_loop$:
    cmp r1,r2
    ldrls r3,[r0],#4
    strls r3,[r1],#4
    bls copy_loop$

    mov r0,r10
    bl main

.global hang
hang: b hang

.globl GETPC
    mov r0,lr
    bx lr

.globl int_enable_irqs
int_enable_irqs:
    cpsie i // interrupt enable: normal interrupts
    bx lr

.globl int_disable_irqs
int_disable_irqs:
    cpsid i
    bx lr

.global dummy
dummy:
    bx lr

.global put32
put32:
    str r1,[r0]
    bx lr

.global get32
get32:
    ldr r0,[r0]
    bx lr

swi:
    b swi

irq:
    b irq

.global __dmb
__dmb:
    mov r0,#0
    mcr p15, 0, r0, c7, c10, 5
    bx lr

/* flush prefetch buffer. used after cp15 access register writes, for instance. */
.global __fpb
__fpb:
    mov r0,#0
    mcr p15, 0, r0, c7, c5, 4
    bx lr
