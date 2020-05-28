#include <os.h>
#include <printf.h>
#include <sched_edf.h>
#include <bsp.h>
#include <timer.h>
#include <runtime_assert.h>
#include <uart.h>

uint32_t deltas[512];
uint32_t measurement_deltas[512];
extern void wait_for_cycles(uint32_t);
extern void collect_deltas();
extern void collect_delta(uint32_t, uint32_t);

static OS_TCB task;
static CPU_STK stack[4096] __attribute__((aligned(8)));

void AppTaskStart(void *p)
{
    (void)p;

    //printf("AppTaskStart called.\n");
    uint32_t idx = 0;

    CPU_SR_ALLOC();

    while(idx++ < sizeof(deltas)/sizeof(deltas[0]))
    {
        uint32_t ts_end;
        CPU_INT_DIS(); // really does save

        {
            __asm__ __volatile__("mcr p15, 0, %0, c15, c12, 0" :: "r"(7));
            __asm__ __volatile__("mrc p15, 0, %0, c15, c12, 1" : "=r"(ts_end));
            measurement_deltas[idx - 1] = ts_end;
        }
        __asm__ __volatile__("mov r0,#640" :::"r0");
        __asm__ __volatile__("mcr p15, 0, %0, c15, c12, 0" :: "r"(7));
        //__asm__ __volatile__("mrc p15, 0, %0, c15, c12, 1" : "=r"(ts_start));
        __asm__ __volatile__("bl wait_for_cycles" ::: "r0", "lr");
        __asm__ __volatile__("mrc p15, 0, %0, c15, c12, 1" : "=r"(ts_end));
        deltas[idx - 1] = ts_end;
        CPU_INT_EN(); // really does restore
        if (idx % 256 == 0)
            uart_sendbyte('.');
        OSFinishInstance();
    }

    printf("Deltas: \n");
    for (size_t i = 0; i < sizeof(deltas)/sizeof(deltas[0]); ++i)
    {
        printf("%ld ", deltas[i]);
    }

    printf("Meas deltas: \n");
    for (size_t i = 0; i < sizeof(deltas)/sizeof(deltas[0]); ++i)
    {
        printf("%ld ", measurement_deltas[i]);
    }

    printf("\nDelta dump complete.");
    hang();
}

void main()
{
    uart_init();
    printf("Starting delta measurements.\n");

    collect_deltas(); // warm up the branch predictor and icache

    OS_ERR err;

    OSInit(&err);
    ASSERT(err == OS_ERR_NONE);

    OSTaskCreate(&task, "Measurement task", &AppTaskStart, 0, &stack[0], 1024, sizeof(stack)/sizeof(stack[0]), 0, 1, 100, 100, 0, 0, &err);

    ASSERT(err == OS_ERR_NONE);

    BSP_Init();
    OSStart(&err);
}
