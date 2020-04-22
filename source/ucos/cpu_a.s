//********************************************************************************************************
//                                                uC/CPU
//                                    CPU CONFIGURATION & PORT LAYER
//
//                          (c) Copyright 2004-2011; Micrium, Inc.; Weston, FL
//
//               All rights reserved.  Protected by international copyright laws.
//
//               uC/CPU is provided in source form to registered licensees ONLY.  It is 
//               illegal to distribute this source code to any third party unless you receive 
//               written permission by an authorized Micrium representative.  Knowledge of 
//               the source code may NOT be used to develop a similar product.
//
//               Please help us continue to provide the Embedded community with the finest 
//               software available.  Your honesty is greatly appreciated.
//
//               You can contact us at www.micrium.com.
//********************************************************************************************************

//********************************************************************************************************
//
//                                            CPU PORT FILE
//
//                                                 ARM
//                                            IAR C Compiler
//                                            Edited for GAS
//
// Filename      : cpu_a.s
// Version       : V1.29.01.00
// Programmer(s) : JJL
//                 JDH
//                 BAN
//                 ITJ
//                 Sam van Kampen
//********************************************************************************************************


//********************************************************************************************************
//                                           GLOBAL FUNCTIONS
//********************************************************************************************************

.global CPU_SR_Save
.global CPU_SR_Restore
.global CPU_IntDis
.global CPU_IntEn
.global CPU_IRQ_Dis
.global CPU_IRQ_En
.global CPU_FIQ_Dis
.global CPU_FIQ_En
.global CPU_CntLeadZeros
.global CPU_WaitForInterrupt

//********************************************************************************************************
//                                                EQUATES
//********************************************************************************************************

.equ CPU_ARM_CTRL_INT_DIS,  0xC0                            // Disable both FIQ & IRQ
.equ CPU_ARM_CTRL_FIQ_DIS,  0x40                            // Disable FIQ.
.equ CPU_ARM_CTRL_IRQ_DIS,  0x80                            // Disable IRQ.

//********************************************************************************************************
//                                      CODE GENERATION DIRECTIVES
//********************************************************************************************************

.section .text


//$PAGE
//********************************************************************************************************
//                                      CRITICAL SECTION FUNCTIONS
//
// Description : Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking, the
//               state of the interrupt disable flag is stored in the local variable 'cpu_sr' & interrupts
//               are then disabled ('cpu_sr' is allocated in all functions that need to disable interrupts).
//               The previous interrupt state is restored by copying 'cpu_sr' into the CPU's status register.
//
// Prototypes  : CPU_SR  CPU_SR_Save   (void)//
//               void    CPU_SR_Restore(CPU_SR  cpu_sr)//
//
// Note(s)     : (1) These functions are used in general like this :
//
//                       void  Task (void  *p_arg)
//                       {
//                           CPU_SR_ALLOC()//                     /* Allocate storage for CPU status register */
//                               :
//                               :
//                           CPU_CRITICAL_ENTER()//               /* cpu_sr = CPU_SR_Save()//                  */
//                               :
//                               :
//                           CPU_CRITICAL_EXIT()//                /* CPU_SR_Restore(cpu_sr)//                  */
//                               :
//                       }
//
//               (2) CPU_SR_Restore() is implemented as recommended by Atmel's application note :
//
//                       "Disabling Interrupts at Processor Level"
//********************************************************************************************************

CPU_SR_Save:
    mrs r0, cpsr     // Get the SR to save it
    cpsid if         // Disable interrupts
    bx lr            // Return CPSR


CPU_SR_Restore:                                                  // See Note #2
    MSR     CPSR_c, R0
    BX      LR


//$PAGE
//********************************************************************************************************
//                                     ENABLE & DISABLE INTERRUPTS
//
// Description : Disable/Enable IRQs & FIQs.
//
// Prototypes  : void  CPU_IntEn (void)//
//               void  CPU_IntDis(void)//
//********************************************************************************************************

CPU_IntDis:
    cpsid if
    bx lr

CPU_IntEn:
    cpsie if
    bx lr


//********************************************************************************************************
//                                        ENABLE & DISABLE IRQs
//
// Description : Disable/Enable IRQs.
//
// Prototypes  : void  CPU_IRQ_En (void)//
//               void  CPU_IRQ_Dis(void)//
//********************************************************************************************************

CPU_IRQ_Dis:
    cpsid i
    bx lr


CPU_IRQ_En:
    cpsie i
    bx lr


//********************************************************************************************************
//                                        ENABLE & DISABLE FIQs
//
// Description : Disable/Enable FIQs.
//
// Prototypes  : void  CPU_FIQ_En (void);
//               void  CPU_FIQ_Dis(void);
//********************************************************************************************************

CPU_FIQ_Dis:
    cpsid f
    bx lr

CPU_FIQ_En:
    cpsie f
    bx lr

// Count leading zeros
CPU_CntLeadZeros:
    clz r0, r0
    bx lr

CPU_WaitForInterrupt:
    mov r0,#0
    mcr p15, 0, r0, c7, c0, 4
    bx lr
