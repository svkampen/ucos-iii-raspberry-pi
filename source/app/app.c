#include <gpio.h>
#include <includes.h>
#include <interrupts.h>
#include <io.h>
#include <timer.h>
#include <uart.h>

static OS_TCB  AppTaskStartTCB;
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];

void AppTaskStart(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    BSP_Init();
    CPU_Init();
    BSP_LED_Off();

    while (1) {
        BSP_LED_Toggle();
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE)
        {
            uart_send("Error in OSTimeDlyHMSM: ");
            printi(err);
        }
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
