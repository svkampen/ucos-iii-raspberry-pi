#include <os.h>
#include <printf.h>
#include <sched_edf.h>
#include <bsp.h>
#include <timer.h>
#include <runtime_assert.h>
#include <uart.h>
#include <tasks.h>

uint32_t deltas[512];
uint32_t measurement_deltas[512];
extern void wait_for_cycles(uint32_t);
extern void collect_deltas();
extern void collect_delta(uint32_t, uint32_t);

//static OS_TCB task;
//static CPU_STK stack[4096] __attribute__((aligned(8)));

#define TICKS_TO_CYCLES(ticks) TICKS_TO_USEC(ticks)*700
#define USEC_TO_CYCLES(usec) (usec)*700

static void reboot_using_watchdog()
{
    volatile u32* PM_RSTC = (vu32*)0x2010001c;
	volatile u32* PM_WDOG = (vu32*)0x20100024;
	const int PM_PASSWORD = 0x5a000000;
	const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

    *PM_WDOG = PM_PASSWORD | 1;
    *PM_RSTC = PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
    hang();
}


void WaitTask(void* arg)
{

    struct task* t = (struct task*)arg;

    while (true)
    {
        if (OSTickCtr > task_set.hyperperiod)
        {
            printf("Hyperperiod passed!\n");
            wait_for_cycles(USEC_TO_CYCLES(1000000));
            __asm__ __volatile__("cpsid if");
            reboot_using_watchdog();
        }
        // allow for a couple usec of slack from task switching overhead
        wait_for_cycles(TICKS_TO_CYCLES(t->wcet) - USEC_TO_CYCLES(5));
#if EDF_CFG_ENABLED
        OSFinishInstance();
#else
        ASSERT(false);
#endif
    }
}

void main()
{
    uart_init();
    printf("Running tasks at %lu\n", OS_TS_GET());

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
        "Task 10"
    };


    for (unsigned i = 0; i < task_set.num_tasks; ++i)
    {
        struct task* t = &task_set.tasks[i];
        OSTaskCreate(&t->tcb, task_names[i], &WaitTask, t, t->stk, 1024, 4096, 0, t->edf_period,
                TICKS_TO_USEC(t->edf_relative_deadline), TICKS_TO_USEC(t->wcet), 0, 0, &err);
        ASSERT(err == OS_ERR_NONE);
    }

    printf("Running task set: %ld tasks, processor utilization %f, hyperperiod %f seconds\n",
           task_set.num_tasks, OSProcessorUtilization(), task_set.hyperperiod/(double)OSCfg_TickRate_Hz);

    BSP_Init();
    OSStart(&err);
}
