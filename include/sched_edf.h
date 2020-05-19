#pragma once
#include <edf_heap.h>

void OSFinishInstance();
#define OS_EDF_FINISH_INSTANCE_NO_BLOCK do { OSTCBCurPtr->EDFCurrentActivationTime += (TICKS_TO_USEC(OSTCBCurPtr->EDFPeriod)); OS_EdfHeapSift(OSTCBCurPtr); } while(0)
#define OS_EDF_RESET_ACTIVATION_TIME do { OSTCBCurPtr->EDFCurrentActivationTime = CPU_TS_TmrRd(); } while(0)

#define SECS_TO_TICKS(secs) (secs * OS_CFG_TICK_RATE_HZ)
#define SECS_TO_USECS(secs) (secs * 1000000)
#define MSECS_TO_USECS(msecs) (msecs * 1000)

