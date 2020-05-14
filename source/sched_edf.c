#include <edf_cfg.h>
#include <edf_heap.h>
#include <printf.h>

#if EDF_CFG_ENABLED

void OSSched (void)
{
    CPU_SR_ALLOC();

    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {
        return;
    }

    if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0)
    {
        return;
    }

    CPU_INT_DIS();
    OSTCBHighRdyPtr = OS_EdfHeapPeek();
    if (OSTCBHighRdyPtr == OSTCBCurPtr)
    {
        CPU_INT_EN();
        return;
    }
#if EDF_CFG_CHECK_DEADLINE_MISS > 0u
    if (EDF_DEADLINE_ABSOLUTE(OSTCBHighRdyPtr) < CPU_TS_TmrRd())
    {
        printf("DEADLINE MISS DETECTED IN TASK %s\n", OSTCBHighRdyPtr->NamePtr);
    }
#endif

    OSTaskCtxSwCtr++;

    OS_TASK_SW();
    CPU_INT_EN();
}

void OSIntExit (void)
{
    CPU_SR_ALLOC();

    if (OSRunning != OS_STATE_OS_RUNNING)
        return;

    CPU_INT_DIS();
    if (OSIntNestingCtr == (OS_NESTING_CTR)0) {
        CPU_INT_EN();
        return;
    }

    if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0) {
        CPU_INT_EN();
        return;
    }

    OSTCBHighRdyPtr = OS_EdfHeapPeek();

}

void OS_TaskBlock (OS_TCB *p_tcb, OS_TICK timeout)
{
    OS_ERR err;
    if (timeout > (OS_TICK)0) {
        OS_TickListInsert(p_tcb, timeout, OS_OPT_TIME_TIMEOUT, &err);
        if (err == OS_ERR_NONE)
            p_tcb->TaskState = OS_TASK_STATE_PEND_TIMEOUT;
        else
            p_tcb->TaskState = OS_TASK_STATE_PEND;
    }
    else
        p_tcb->TaskState = OS_TASK_STATE_PEND;
    OS_EdfHeapRemove(p_tcb);
}

void OS_TaskRdy (OS_TCB *p_tcb)
{
    OS_ERR err;
    OS_TickListRemove(p_tcb);
    if ((p_tcb->TaskState & OS_TASK_STATE_BIT_SUSPENDED) == (OS_STATE)0) {
        OS_EdfHeapInsert(p_tcb, &err);
        if (err != OS_ERR_NONE)
            printf("Unable to ready task %s: EDF heap full!\n", p_tcb->NamePtr);
    }
}
#endif
