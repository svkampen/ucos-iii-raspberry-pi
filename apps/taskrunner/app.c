#include <os.h>
#include <printf.h>
#include <sched_edf.h>
#include <bsp.h>
#include <timer.h>
#include <runtime_assert.h>
#include <uart.h>
#include <tasks.h>

extern void wait_for_cycles(uint32_t);

//static OS_TCB task;
//static CPU_STK stack[4096] __attribute__((aligned(8)));

#define TICKS_TO_CYCLES(ticks) TICKS_TO_USEC(ticks)*700
#define USEC_TO_CYCLES(usec) (usec)*700

void reboot_using_watchdog()
{
    volatile u32* PM_RSTC = (vu32*)0x2010001c;
    volatile u32* PM_WDOG = (vu32*)0x20100024;
    const int PM_PASSWORD = 0x5a000000;
    const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

    *PM_WDOG = PM_PASSWORD | 1;
    *PM_RSTC = PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
    hang();
}

void TickTaskFinishInstance()
{
    CPU_SR_ALLOC();


    OS_CRITICAL_ENTER();
    OS_TCB* const tcb = OSTCBCurPtr;
    const uint64_t relative_deadline = 250; // us
    const uint64_t time = CPU_TS_Get64();
    const uint64_t absolute_deadline = tcb->CurrentActivationTime + relative_deadline;

    if (absolute_deadline < time)
    {
        printf("Warning: deadline for task `%s' missed: %llu < %llu, exiting...\n",
                tcb->NamePtr, absolute_deadline, time);
        printf("Hyperperiod passed!\n");
        wait_for_cycles(USEC_TO_CYCLES(1000000));
        reboot_using_watchdog();
    }

    tcb->CurrentActivationTime += TICKS_TO_USEC(1);
    OS_CRITICAL_EXIT();
}

void RMFinishInstance(struct task* const t, OS_TCB* const tcb)
{
    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();
    uint64_t relative_deadline = TICKS_TO_USEC(t->edf_relative_deadline);
    uint64_t time = CPU_TS_Get64();
    uint64_t absolute_deadline = tcb->CurrentActivationTime + relative_deadline;
    CPU_TS64 delta = time - tcb->CurrentActivationTime;
    OS_TICK elapsed_ticks = (delta * OSCfg_TickRate_Hz) / 1000000;
    CPU_INT32S ticks_left = t->edf_period - elapsed_ticks;

    WARN_IF_NOT(ticks_left > 0);

    if (absolute_deadline < time)
    {
        printf("Warning: deadline for task `%s' missed: %llu < %llu, exiting...\n",
                OSTCBCurPtr->NamePtr, absolute_deadline, time);
        printf("Hyperperiod passed!\n");
        wait_for_cycles(USEC_TO_CYCLES(1000000));
        reboot_using_watchdog();
    }

    WARN_IF_NOT_OP(tcb->CurrentActivationTime, <=, time);

    tcb->CurrentActivationTime += TICKS_TO_USEC(t->edf_period);

    OS_ERR err;

    OS_CRITICAL_EXIT();
    OSTimeDly(ticks_left, OS_OPT_TIME_DLY, &err);
    ASSERT(err == OS_ERR_NONE);
}

void WaitTask(void* arg)
{

    struct task* t = (struct task*)arg;
#if !(EDF_CFG_ENABLED)
    OS_TCB* tcb = &t->tcb;
#endif

    while (true)
    {
        if (OSTickCtr > task_set.hyperperiod)
        {
            __asm__ __volatile__("cpsid if");
            printf("Hyperperiod passed!\n");
            wait_for_cycles(USEC_TO_CYCLES(1000000));
            reboot_using_watchdog();
        }
        // allow for a couple usec of slack from task switching overhead
        wait_for_cycles(TICKS_TO_CYCLES(t->wcet) - USEC_TO_CYCLES(2));
#if EDF_CFG_ENABLED
        OSFinishInstance();
#else
        RMFinishInstance(t, tcb);
#endif
    }
}

void main()
{
    uart_init();
    printf("Running tasks at %llu\n", CPU_TS_TmrRd());

    OS_ERR err;

    OSInit(&err);
    ASSERT(err == OS_ERR_NONE);

    char* task_names[] = {
        "Task 1",
        "Task 2",
        "Task 3",
        "Task 4",
        "Task 5",
        "Task 6",
        "Task 7",
        "Task 8",
        "Task 9",
        "Task 10",
        "Task 11",
        "Task 12",
        "Task 13",
        "Task 14",
        "Task 15",
        "Task 16"
    };

    double processor_utilization = 0.0;

    for (unsigned i = 0; i < task_set.num_tasks; ++i)
    {
        struct task* t = &task_set.tasks[i];
        processor_utilization += (t->wcet / (double)t->edf_period);
#if EDF_CFG_ENABLED > 0u
        OSTaskCreate(&t->tcb, task_names[i % 16], &WaitTask, t, t->stk, 1024, 2048, 0, t->edf_period,
                TICKS_TO_USEC(t->edf_relative_deadline), TICKS_TO_USEC(t->wcet), 0, 0, &err);
#else
        OSTaskCreate(&t->tcb, task_names[i % 16], &WaitTask, t, t->rm_priority, t->stk,
                     1024, 2048, 0, 0, 0, 0, &err);
#endif
        ASSERT(err == OS_ERR_NONE);
    }

    printf("Running task set: %ld tasks, processor utilization %f, hyperperiod %f seconds\n",
           task_set.num_tasks, processor_utilization, task_set.hyperperiod/(double)OSCfg_TickRate_Hz);

    BSP_Init();
#if EDF_CFG_ENABLED == 0u
    uint64_t time = CPU_TS_Get64();
    for (unsigned i = 0; i < task_set.num_tasks; ++i)
    {
         task_set.tasks[i].tcb.CurrentActivationTime = time;
    }
    OSTickTaskTCB.CurrentActivationTime = time;
    OSIdleTaskTCB.CurrentActivationTime = time;
#endif
    OSStart(&err);
    __builtin_unreachable();
}
