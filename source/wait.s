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
