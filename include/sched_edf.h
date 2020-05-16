#pragma once

void OSFinishInstance();
#define OS_EDF_FINISH_INSTANCE_NO_BLOCK do { OSTCBCurPtr->EDFLastActivationTime += (TICKS_TO_USEC(OSTCBCurPtr->EDFPeriod)); OS_EdfHeapSift(OSTCBCurPtr); } while(0)
#define OS_EDF_RESET_ACTIVATION_TIME do { OSTCBCurPtr->EDFLastActivationTime = CPU_TS_TmrRd(); } while(0)
