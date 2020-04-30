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

void DoNothingTask(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;

    do {
        OSSchedRoundRobinYield(&err);
        if (err != OS_ERR_NONE) ERR("OSSchedRoundRobinYield", err);
    } while (DEF_ON);
}

void AppTaskStart(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    timestamp = 0;
    switches = 0;
    samples = 0;

    BSP_Init();
    CPU_Init();
    BSP_LED_Off();

    OSSchedRoundRobinCfg(DEF_ENABLED, 0, &err);
    if (err != OS_ERR_NONE)
        ERR("OSSchedRoundRobinCfg", err);

    OSStatTaskCPUUsageInit(&err);
    if (err != OS_ERR_NONE)
        ERR("CPU usage init", err);

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
        BSP_LED_Toggle();
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE)
        {
            ERR("OSTimeDlyHMSM", err);
        }
        printf("CPU usage as measured by stat task (in tienduizendsten): %d\n", OSStatTaskCPUUsage);
        printf("Context switches performed: %d\n", OSTaskCtxSwCtr);
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
