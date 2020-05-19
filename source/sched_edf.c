#include <edf_cfg.h>
#include <edf_heap.h>
#include <printf.h>
#include <runtime_assert.h>

extern void hang();

#if EDF_CFG_ENABLED

void OSSched(void)
{
    CPU_SR_ALLOC();

    if (OSIntNestingCtr > (OS_NESTING_CTR)0)
    {
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

    printf("OSSched: switching to %s\n", OSTCBHighRdyPtr->NamePtr);

#if EDF_CFG_CHECK_DEADLINE_MISS > 0u
    WARN_IF_NOT_OP(EDF_DEADLINE_ABSOLUTE(OSTCBHighRdyPtr), >=, CPU_TS_TmrRd());
#endif

    OSTaskCtxSwCtr++;

    OS_TASK_SW();
    CPU_INT_EN();
}

void OSIntExit(void)
{
    CPU_SR_ALLOC();

    if (OSRunning != OS_STATE_OS_RUNNING) return;

    CPU_INT_DIS();
    if (OSIntNestingCtr == (OS_NESTING_CTR)0)
    {
        CPU_INT_EN();
        return;
    }

    if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0)
    {
        CPU_INT_EN();
        return;
    }

    OSTCBHighRdyPtr = OS_EdfHeapPeek();
    OSIntCtxSw();
    CPU_INT_EN();
}

void OS_TaskBlock(OS_TCB* p_tcb, OS_TICK timeout)
{
    OS_ERR err;
    printf("OS_TaskBlock: %s\n", p_tcb->NamePtr);
    if (timeout > (OS_TICK)0)
    {
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

void OS_TaskRdy(OS_TCB* p_tcb)
{
    OS_ERR err;
    printf("OS_TaskRdy: %s\n", p_tcb->NamePtr);
    OS_TickListRemove(p_tcb);
    if ((p_tcb->TaskState & OS_TASK_STATE_BIT_SUSPENDED) == (OS_STATE)0)
    {
        OS_EdfHeapInsert(p_tcb, &err);
        ASSERT(err == OS_ERR_NONE);
    }
}

void OSTaskCreate(OS_TCB* p_tcb, CPU_CHAR* p_name, OS_TASK_PTR p_task,
                  void* p_arg, CPU_STK* p_stk_base, CPU_STK_SIZE stk_limit,
                  CPU_STK_SIZE stk_size, OS_MSG_QTY q_size, OS_TICK period,
                  CPU_TS64 relative_deadline, CPU_TS64 wcet, void* p_ext,
                  OS_OPT opt, OS_ERR* err)
{
    CPU_STK_SIZE i;
    CPU_STK*     p_sp;
    CPU_STK*     p_stk_limit;
    OS_REG_ID    reg_nbr;

    CPU_SR_ALLOC();

    OS_TaskInitTCB(p_tcb);

    *err = OS_ERR_NONE;

    /* --------------- CLEAR THE TASK'S STACK --------------- */
    if ((opt & OS_OPT_TASK_STK_CHK) != (OS_OPT)0)
    { /* See if stack checking has been enabled                 */
        if ((opt & OS_OPT_TASK_STK_CLR) != (OS_OPT)0)
        { /* See if stack needs to be cleared                       */
            p_sp = p_stk_base;
            for (i = 0u; i < stk_size; i++)
            { /* Stack grows from HIGH to LOW memory                    */
                *p_sp = (CPU_STK)0; /* Clear from bottom of stack and up! */
                p_sp++;
            }
        }
    }

#if (CPU_CFG_STK_GROWTH == CPU_STK_GROWTH_HI_TO_LO)
    p_stk_limit = p_stk_base + stk_limit;
#else
    p_stk_limit = p_stk_base + (stk_size - 1u) - stk_limit;
#endif

    p_sp = OSTaskStkInit(p_task, p_arg, p_stk_base, p_stk_limit, stk_size, opt);

    p_tcb->TaskEntryAddr = p_task; /* Save task entry point address */
    p_tcb->TaskEntryArg  = p_arg;  /* Save task entry argument  */

    p_tcb->NamePtr = p_name; /* Save task name */

    p_tcb->StkPtr      = p_sp; /* Save the new top-of-stack pointer        */
    p_tcb->StkLimitPtr = p_stk_limit; /* Save the stack limit pointer */

    p_tcb->ExtPtr = p_ext; /* Save pointer to TCB extension */
    p_tcb->StkBasePtr =
        p_stk_base; /* Save pointer to the base address of the stack          */
    p_tcb->StkSize =
        stk_size; /* Save the stack size (in number of CPU_STK elements)    */
    p_tcb->Opt = opt; /* Save task options */

    p_tcb->EDFPeriod             = period;
    p_tcb->EDFRelativeDeadline   = relative_deadline;
    p_tcb->EDFWorstCaseExecutionTime = wcet;
    p_tcb->EDFLastActivationTime = 0;

#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    for (reg_nbr = 0u; reg_nbr < OS_CFG_TASK_REG_TBL_SIZE; reg_nbr++)
    {
        p_tcb->RegTbl[reg_nbr] = (OS_REG)0;
    }
#endif

#if OS_CFG_TASK_Q_EN > 0u
    OS_MsgQInit(&p_tcb->MsgQ, /* Initialize the task's message queue */
                q_size);
#else
    (void)&q_size;
#endif

    OSTaskCreateHook(p_tcb); /* Call user defined hook */

#if defined(OS_CFG_TLS_TBL_SIZE) && (OS_CFG_TLS_TBL_SIZE > 0u)
    for (id = 0u; id < OS_CFG_TLS_TBL_SIZE; id++)
    {
        p_tcb->TLS_Tbl[id] = (OS_TLS)0;
    }
    OS_TLS_TaskCreate(p_tcb); /* Call TLS hook */
#endif
    /* --------------- ADD TASK TO READY LIST --------------- */
    OS_CRITICAL_ENTER();

    OS_TaskRdy(p_tcb);

#if OS_CFG_DBG_EN > 0u
    OS_TaskDbgListAdd(p_tcb);
#endif

    OSTaskQty++; /* Increment the #tasks counter                           */

    if (OSRunning != OS_STATE_OS_RUNNING)
    { /* Return if multitasking has not started                 */
        OS_CRITICAL_EXIT();
        return;
    }

    OS_CRITICAL_EXIT_NO_SCHED();

    OSSched();
}

void OSFinishInstance()
{
    OS_TCB* p_tcb = OSTCBCurPtr;

    CPU_SR_ALLOC();

    OS_CRITICAL_ENTER();

    CPU_TS time  = CPU_TS_TmrRd();
    CPU_TS delta = time - p_tcb->EDFLastActivationTime;
    /* timer resolution = 1 μs, and 1 s = 1e6 μs */
    /* FIXME might be off by at most one tick? but should sync as tasks activate
     * periodically since the activation time will be at a tick */
    OS_TICK    elapsed_ticks = (delta * OSCfg_TickRate_Hz) / 1000000;
    CPU_INT32S ticks_left    = p_tcb->EDFPeriod - elapsed_ticks;
    ASSERT(ticks_left > 0);

    OS_TaskBlock(p_tcb, ticks_left);

    p_tcb->EDFLastActivationTime += (TICKS_TO_USEC(p_tcb->EDFPeriod));

    OS_CRITICAL_EXIT_NO_SCHED();

    OSSched();
}

#endif
