#include <edf_heap.h>
#include <includes.h>
#include <printf.h>
#include <sched_edf.h>
#include <uart.h>

static OS_TCB  AppTaskStartTCB;
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE] __attribute__ ((aligned (8)));

volatile uint32_t* timer32 = (volatile uint32_t*)0x20003004;

#define ERR(func, err) printf(func " returned error: %08x\n", err);
#define CHECK_ERR(func)                                                        \
    if (err != OS_ERR_NONE)                                                    \
    {                                                                          \
        ERR(func, err);                                                        \
    }

void AppTaskStart(void* p_arg)
{
    OS_ERR err = OS_ERR_NONE;
    (void)p_arg;

    BSP_Init();
    CPU_Init();
    BSP_LED_Off();

    while (DEF_ON)
    {
        CHECK_ERR("OSTimeDlyHMSM");
        printf("In main task. EDF heap: ");
        OS_EdfPrintHeap();
        OSFinishInstance();
    }
}

void main(void)
{
    OS_ERR err;

    __dmb();

    uart_init();
    printf("\nStarting EDF test task.\n");

    OSInit(&err);
    CHECK_ERR("OSInit");

    OSTaskCreate(&AppTaskStartTCB, "Main task", AppTaskStart, 0,
                 AppTaskStartStk, APP_TASK_START_STK_SIZE / 10,
                 APP_TASK_START_STK_SIZE, 0, 5, SECS_TO_USECS(1),
                 MSECS_TO_USECS(100), 0, 0, &err);

    CHECK_ERR("OSTaskCreate");

    OSStart(&err);
    CHECK_ERR("OSStart");
}
