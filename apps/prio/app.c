#include <includes.h>
#include <printf.h>
#include <uart.h>

static OS_TCB  AppTaskTCBs[APP_PRIO_TASKS];
static CPU_STK AppTaskStks[APP_PRIO_STK_SIZE * APP_PRIO_TASKS];

#define NUM_DELTAS 2048000
static CPU_TS32   timestamp;
static CPU_TS32   deltas[NUM_DELTAS];
static uint32_t ts_idx;

static OS_TCB  AppTaskStartTCB;
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];

volatile uint32_t* timer32 = (volatile uint32_t*)0x20003004;

#define ERR(func, err) printf(func " returned error: %08x\n", err);
#define CHECK_ERR(func)                                                        \
    if (err != OS_ERR_NONE)                                                    \
    {                                                                          \
        ERR(func, err);                                                        \
    }

void DoNothingTask(void* p_arg)
{
    OS_ERR   err;
    uint32_t task_idx = (uint32_t)p_arg;
    uint32_t wake_up_tsk;
    CPU_TS32 new_ts;

    if (timestamp == 0) goto yield;

    do
    {
        new_ts  = *timer32;
        if (ts_idx >= NUM_DELTAS) goto yield;
        deltas[ts_idx++] = new_ts - timestamp;

yield:
        timestamp = *timer32;

        wake_up_tsk = (task_idx + 1) % APP_PRIO_TASKS;

        OSTaskResume(&AppTaskTCBs[wake_up_tsk], &err);
        if (err != OS_ERR_NONE && err != OS_ERR_TASK_NOT_SUSPENDED)
            ERR("OSTaskResume", err);

        /* If task_idx == APP_PRIO_TASKS - 1, we're the lowest-priority task.
         * We don't need to suspend (and in fact can't - we've already been
         * pre-empted because we woke up the highest-priority task), because
         * we can't pre-empt anyone. */
        if (task_idx == APP_PRIO_TASKS - 1) continue;
        OSTaskSuspend(NULL, &err);
        if (err != OS_ERR_NONE)
            printf("Task ID %ld: ts_idx %ld: OSTaskSuspend errored: %d\n", task_idx, ts_idx, err);
    } while (DEF_ON);
}

void AppTaskStart(void* p_arg)
{
    OS_ERR err;
    (void)p_arg;

    BSP_Init();
    CPU_Init();
    BSP_LED_Off();

    for (uint32_t i = 0; i < APP_PRIO_TASKS; ++i)
    {
        uint32_t priority = APP_PRIO_BASE_PRIO + (i * APP_PRIO_DELTA_PER_TASK);
        printf("Creating task %ld (priority %ld)...\n", i, priority);
        OSTaskCreate(&AppTaskTCBs[i], "Prio task", DoNothingTask, (void*)i,
                     priority, &AppTaskStks[i * APP_PRIO_STK_SIZE], 0,
                     APP_PRIO_STK_SIZE, 0, 0, 0, OS_OPT_TASK_NONE, &err);
        CHECK_ERR("OSTaskCreate");
    }

    printf("Starting measurements...\n");

    ts_idx = 0;
    timestamp = 0;

    while (DEF_ON)
    {
        OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        CHECK_ERR("OSTimeDlyHMSM");

        printf("Starting delta dump: %ld deltas\n", ts_idx);

        for (uint32_t i = 0; i < ts_idx; ++i)
        {
            printf("%ld ", deltas[i]);
        }
        printf("\n");
        ts_idx = 0;
    }
}

void main(void)
{
    OS_ERR err;

    __dmb();

    uart_init();
    printf("\nStarting huge priority delta measurement task.\n");

    OSInit(&err);
    CHECK_ERR("OSInit");

    OSTaskCreate(&AppTaskStartTCB, "Main task", AppTaskStart, 0,
                 APP_TASK_START_PRIO, AppTaskStartStk, 0,
                 APP_TASK_START_STK_SIZE, 0, 0, 0, OS_OPT_TASK_NONE, &err);

    CHECK_ERR("OSTaskCreate");

    OSStart(&err);
}
