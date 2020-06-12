#pragma once
#include <edf_cfg.h>
#include <edf_heap.h>

void OSFinishInstance();
double OSProcessorUtilization();
#define OS_EDF_FINISH_INSTANCE_NO_BLOCK do { CPU_SR_ALLOC(); OS_CRITICAL_ENTER(); OSTCBCurPtr->CurrentActivationTime += OSTCBCurPtr->EDFPeriod; OS_EdfHeapSift(OSTCBCurPtr); OS_CRITICAL_EXIT();} while(0)
#if EDF_CFG_ENABLED
#define OS_RESET_ACTIVATION_TIME do { CPU_SR_ALLOC(); OS_CRITICAL_ENTER(); OSTCBCurPtr->CurrentActivationTime = CPU_TS_TmrRd(); OS_EdfHeapSift(OSTCBCurPtr); OS_CRITICAL_EXIT(); } while(0)
#else
#define OS_RESET_ACTIVATION_TIME do { CPU_SR_ALLOC(); OS_CRITICAL_ENTER(); OSTCBCurPtr->CurrentActivationTime = CPU_TS_TmrRd(); OS_CRITICAL_EXIT(); } while(0)
#endif

#define TICKS_TO_USEC(ticks) ((1000000 * ticks) / OSCfg_TickRate_Hz)
#define SECS_TO_TICKS(secs) (secs * OSCfg_TickRate_Hz)
#define USECS_TO_TICKS(usecs) ((usecs * OSCfg_TickRate_Hz)/1000000)
#define SECS_TO_USECS(secs) (secs * 1000000)
#define MSECS_TO_USECS(msecs) (msecs * 1000)

