.section .text
.global wait_for_cycles

.extern uart_init
.extern deltas

// Waits for r0 + 4/5 (return) + 6 (mispredict) + 0/3 (static branch)
// (r0 + 10 or +11 or +13 or +14)
// Preconditions: r0 > 0
wait_for_cycles:
    lsr r0,r0,#1        // r0 /= 2
wfc_loop$:
    subs r0, r0, #1     // 1 cycle
    bne wfc_loop$       // 1 cycle (4 eerste keer program-wide (static), 7 laatste keer (mispredict))
end$:
    bx lr               // 4/5 cycles, we don't edit lr but the return stack may be empty because it was used during an interrupt

.global collect_delta
collect_delta:
    push {r6,r7,r8,lr}
    // r0 = #c to wait
    // r1 = IDX
    lsl r1,r1,#2
    mov r3,#7

    mcr p15, 0, r3, c15, c12, 0 // reset counters and enable them
    mrc p15, 0, r7, c15, c12, 1 //read
    bl wait_for_cycles
    mrc p15, 0, r8, c15, c12, 1 //read
    sub r7,r8,r7
    str r7,[r6, r1]

    pop {r6,r7,r8,pc}


.global collect_deltas
collect_deltas:
    mov r2,lr
    ldr r3,=#1024000
    ldr r5,=#0x140
    ldr r6,=deltas
    mov r1,#0b111
    mov r0,r5
    mcr p15, 0, r1, c15, c12, 0 // reset counters, enable them
loop$:
    mrc p15, 0, r7, c15, c12, 1 // read cycle counter
    bl wait_for_cycles // 1 cycle
    mrc p15, 0, r8, c15, c12, 1 // read cycle counter
    subs r3,r3,#1
    sub r7,r8,r7
    str r7, [r6], #4
    mov r0,r5
    bne loop$
    bx r2

