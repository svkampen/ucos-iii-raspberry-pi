.extern __bss_start
.extern __bss_end
.extern main
.extern int_handle_irq
.extern OSTCBCurPtr
.extern OSIntNestingCtr
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
    mov r10,r1

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

    // System/user mode: stack at 0x4000
    cps #31
    mov sp,#0x4000


    // SVC mode: stack at 0x8000000 (128 MiB)
    mov r0,#0xD3
    msr cpsr_c,r0
    mov sp,#0x8000000

    bl int_disable_irqs

    // set up FPU access
    // The FPU is a coprocessor, using coprocessor numbers 10 and 11.
    // 10 is used for single-precision floating-point arithmetic,
    // 11 is used for double-precision floating-point arithmetic.
    // We need to allow access to them through the system control
    // coprocessor - CP15.
    // mrc p15, 0, r0, c1, c0, 2
    // orr r0, r0, #0x300000 // allow access to CP10 in privileged and user mode
    // orr r0, r0, #0xC00000 // allow access to CP11 in privileged and user mode
    // mcr p15, 0, r0, c1, c0, 2
    // According to the ARM1176-JZF-S technical reference manual, we need to
    // flush the prefetch buffer here (after any writes to the coprocessor
    // access register). Unsurprisingly that's a coprocessor write, too.
    // bl __fpb

    // Set the FPU enable bit in the floating point exception register.
    // mov r0,#0x40000000
    // fmxr fpexc,r0


// Zero out C BSS

    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0

zero_loop$:
    cmp r0,r1
    strls r2,[r0],#4
    bls zero_loop$

    mov r1, r10

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

.global data_memory_barrier
data_memory_barrier:
    mov r0,#0
    mcr p15, 0, r0, c7, c10, 5
    bx lr

swi:
    b swi

irq:
    OS_IRQ_CTX_SAVE // OS_CTX_SAVE, but saves SPSR_irq instead of CPSR

    teq r1,#1            // if (OSIntNestingCtr == 1)
    bne irq_nested$      // if nestingctr > 1, this irq is nested
                         // i.e. we're not interrupting a task

    ldr r0, =OSTCBCurPtr // OSTCBCurPtr->StkPtr = SP;
    ldr r1, [r0]
    str sp, [r1] 


irq_nested$:

    bl int_handle_irq

    OS_CTX_RESTORE

    //push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    //bl OS_CPU_IRQ_ISR
    //pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    //subs pc,lr,#4

/* Data Memory Barrier. As described in the BCM2835 peripherals manual, required between accesses of different peripherals.
 * Memory accesses occurring in program order before this operation are globally observed before memory accesses hereafter in
 * program order.
 */
.global __dmb
__dmb:
    mov r0,#0
    mcr p15, 0, r0, c7, c10, 5
    bx lr

/* Flush Prefetch Buffer. Used after CP15 access register writes, for instance. */
.global __fpb
__fpb:
    mov r0,#0
    mcr p15, 0, r0, c7, c5, 4
    bx lr
