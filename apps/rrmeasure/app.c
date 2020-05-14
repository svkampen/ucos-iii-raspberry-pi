#include <gpio.h>
#include <includes.h>
#include <interrupts.h>
#include <timer.h>
#include <uart.h>
#include <printf.h>

#define ERR(func, err) printf(func " returned error: %08x\n", err);

static OS_TCB  AppRoundRobinTaskTCBs[APP_ROUND_ROBIN_TASKS];
static CPU_STK AppRoundRobinTaskStks[APP_ROUND_ROBIN_STK_SIZE * APP_ROUND_ROBIN_TASKS];

static OS_TCB  AppTaskStartTCB;
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];

static CPU_INT32U samples;
static CPU_TS32 timestamp;
static CPU_INT32U switches;

volatile uint32_t* timer32 = (volatile uint32_t*)0x20003004;

static CPU_TS32 deltas[1024000];
static CPU_INT32U ts_idx;

void DoNothingTask(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;

    if (timestamp == 0) timestamp = *timer32;

    do {
        CPU_TS32 new_ts = *timer32;
        if (ts_idx > sizeof(deltas)/sizeof(deltas[0])) goto yield;
        deltas[ts_idx++] = new_ts - timestamp;

yield:
        timestamp = *timer32;
        OSSchedRoundRobinYield(&err);
        if (err != OS_ERR_NONE) ERR("OSSchedRoundRobinYield", err);
    } while (DEF_ON);
}

void AppTaskStart(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    ts_idx = 0;
    timestamp = 0;

    BSP_Init();
    CPU_Init();
    BSP_LED_Off();

    OSSchedRoundRobinCfg(DEF_ENABLED, 0, &err);
    if (err != OS_ERR_NONE)
        ERR("OSSchedRoundRobinCfg", err);

    for (int i = 0; i < APP_ROUND_ROBIN_TASKS; ++i)
    {
        OSTaskCreate(&AppRoundRobinTaskTCBs[i],
                     "Round Robin task",
                     DoNothingTask,
                     0,
                     APP_ROUND_ROBIN_PRIO,
                     &AppRoundRobinTaskStks[i * APP_ROUND_ROBIN_STK_SIZE],
                     0,
                     APP_ROUND_ROBIN_STK_SIZE,
                     0,
                     0,
                     0,
                     OS_OPT_TASK_NONE,
                     &err);

        if (err != OS_ERR_NONE)
        {
            ERR("OSTaskCreate (round-robin)", err);
        }
    }

    printf("Starting measurements...\n");

    while (1) {
        OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE)
        {
            ERR("OSTimeDlyHMSM", err);
        }
        printf("Starting delta dump: %d deltas\n", ts_idx);
        for (int i = 0; i < ts_idx; ++i)
        {
            printf("%d ", deltas[i]);
        }
        printf("\n");
        timestamp = *timer32;
        ts_idx = 0;
    }
}

void main(void)
{
    OS_ERR err;

    __dmb();

    uart_init();
    printf("\nInitialized UART. You should be able to see this.\n");

    OSInit(&err);

    if (err != OS_ERR_NONE)
    {
        ERR("OSInit", err);
    }
    else
        printf("OSInit complete\n");

    OSTaskCreate((OS_TCB*)       &AppTaskStartTCB,
                 (CPU_CHAR*)     "App Task Start",
                 (OS_TASK_PTR)   AppTaskStart,
                 (void*)         0,
                 (OS_PRIO)       APP_TASK_START_PRIO,
                 (CPU_STK*)      &AppTaskStartStk[0],
                 (CPU_STK_SIZE)  APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)  APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY)    0,
                 (OS_TICK)       0,
                 (void*)         0,
                 (OS_OPT)        OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR*)       &err);

    if (err != OS_ERR_NONE)
    {
        ERR("OSTaskCreate", err);
    }
    else
        printf("OSTaskCreate complete.\n");

    OSStart(&err);
}
