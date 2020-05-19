.equ ARM_MODE_SVC,0x13

.macro OS_IRQ_CTX_SAVE
    // Store LR, SPSR_irq on the supervisor mode stack
    // We subtract four here because lr would point to the instruction after
    // the interrupted one, and we want to restart the interrupted one.
    sub lr, lr, #4
    srsfd sp!,#ARM_MODE_SVC

    cps #ARM_MODE_SVC
    push {r0-r12}

    and r1, sp, #4
    sub sp, sp, r1
    push {r1, lr}
    vpush {d0-d15}
.endm

.macro OS_CTX_SAVE
    str lr,[sp,#-8]
    mrs lr, cpsr
    push {lr}
    sub sp,sp,#4
    push {r0-r12}
    /* Align stack to 8 bytes */

    and r1, sp, #4 // r1 contains 'delta' between 8-byte-aligned stack and interrupted stack
    sub sp, sp, r1
    push {r1, lr} // make sure we store 8 bytes >.>
    vpush {d0-d15}
.endm

.macro OS_CTX_RESTORE
    vpop {d0-d15} // au. 4*32=128 bytes aan stackruimte...
    pop {r1, lr}
    add sp, sp, r1 // Restore stack alignment

    pop {r0-r12}
    rfefd sp!
.endm

.macro OS_ISR_INCNESTCTR
    ldr r0, =OSIntNestingCtr
    ldrb r1, [r0]
    add r1, r1, #1
    strb r1, [r0]
.endm
