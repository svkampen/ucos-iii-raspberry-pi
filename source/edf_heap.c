#include <edf_cfg.h>
#include <includes.h>
#include <printf.h>
#include <runtime_assert.h>
#include <inttypes.h>

#if EDF_CFG_ENABLED
int OS_EdfHeapCompare(OS_TCB* a, OS_TCB* b)
{
    CPU_TS64 aAbsDeadline = EDF_DEADLINE_ABSOLUTE(a);
    CPU_TS64 bAbsDeadline = EDF_DEADLINE_ABSOLUTE(b);
    return (aAbsDeadline > bAbsDeadline) ? 1 : ((aAbsDeadline < bAbsDeadline) ? -1 : 0);
}

OS_TCB* OS_EdfHeapPeek(void)
{
    return OSEdfHeap[0];
}

void print_task_name_deadline(int idx, OS_TCB* task)
{
    printf("%d %s P %lu RD %llx LAT %llx", idx, task->NamePtr, task->EDFPeriod, task->EDFRelativeDeadline, task->EDFCurrentActivationTime);
}

void OS_EdfResetActivationTimes()
{
    for (uint32_t i = 0; i < OSEdfHeapSize; ++i)
    {
        OSEdfHeap[i]->EDFCurrentActivationTime = CPU_TS_TmrRd();
    }
}

void OS_EdfHeapSiftUp(int index)
{
    int   compare_result, parent_idx;
    OS_TCB* tmp;

    if (index == 0) return;

    parent_idx     = (index - 1) / 2;
    compare_result = OS_EdfHeapCompare(OSEdfHeap[index], OSEdfHeap[parent_idx]);

    while (compare_result < 0)
    {
        tmp              = OSEdfHeap[index];
        OSEdfHeap[index]      = OSEdfHeap[parent_idx];
        OSEdfHeap[parent_idx] = tmp;

        tmp->EDFHeapIndex = parent_idx;
        OSEdfHeap[index]->EDFHeapIndex = index;

        index = parent_idx;

        parent_idx     = (index - 1) / 2;
        compare_result = OS_EdfHeapCompare(OSEdfHeap[index], OSEdfHeap[parent_idx]);
    }
}

void print_helper(unsigned idx)
{
    if (idx >= OSEdfHeapSize) return;
    printf("Node ");
    print_task_name_deadline(idx, OSEdfHeap[idx]);
    printf(" Left (");
    print_helper(2 * idx + 1);
    printf(") Right (");
    print_helper(2 * idx + 2);
    printf(")");
}

void OS_EdfPrintHeap()
{
    print_helper(0);
    printf("\n");
}

void OS_EdfHeapSiftDown(int index)
{
    uint32_t left_index, right_index;
    OS_TCB *element, *left_element, *right_element;
    int l_or_r_swap;

    while (DEF_ON)
    {
        left_index  = 2 * index + 1;
        right_index = left_index + 1;

        element = OSEdfHeap[index];

        /* We're at the end of our used heap. */
        if (left_index >= OSEdfHeapSize) break;
        left_element  = OSEdfHeap[left_index];
        right_element = OSEdfHeap[right_index];

        /* I've forgotten exactly what the assignment is for..
         * maybe the compare later on, to force one side of the || ? */
        if (left_element == NULL)
        {
            l_or_r_swap  = 1;
            left_element = element;
        }
        else if (right_element == NULL)
        {
            l_or_r_swap   = -1;
            right_element = element;
        }
        else
            l_or_r_swap = OS_EdfHeapCompare(left_element, right_element);

        if (OS_EdfHeapCompare(element, left_element) > 0 ||
            OS_EdfHeapCompare(element, right_element) > 0)

        {
            if (l_or_r_swap < 0)
            {
                OSEdfHeap[left_index] = element;
                element->EDFHeapIndex = left_index;
                OSEdfHeap[index]      = left_element;
                left_element->EDFHeapIndex = index;
                index            = left_index;
            }
            else
            {
                OSEdfHeap[right_index] = element;
                element->EDFHeapIndex = right_index;
                OSEdfHeap[index]       = right_element;
                right_element->EDFHeapIndex = index;
                index             = right_index;
            }
        }
        else
            break;
    }
}

void OS_EdfHeapInit(void)
{
    OSEdfHeapSize = 0;
    for (unsigned i = 0; i < EDF_CFG_MAX_TASKS; ++i)
    {
        OSEdfHeap[i] = NULL;
    }
}

/*
 * Sift an element into place in the heap. Can be used when, for instance,
 * a task deadline has changed without it being removed from the heap (since
 * removal and insertion automatically sift).
 */
void OS_EdfHeapSift(OS_TCB* p_tcb)
{
    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();

    uint32_t idx = p_tcb->EDFHeapIndex;

    int parent_idx = (idx - 1) / 2;
    /* Sift down if greater than or equal to parent, up otherwise. */
    if (OS_EdfHeapCompare(OSEdfHeap[idx], OSEdfHeap[parent_idx]) >= 0)
        OS_EdfHeapSiftDown(idx);
    else
        OS_EdfHeapSiftUp(idx);

    OS_CRITICAL_EXIT();
}

void OS_EdfHeapInsert(OS_TCB* p_tcb, OS_ERR* p_err)
{
    if (OSEdfHeapSize == EDF_CFG_MAX_TASKS)
    {
        *p_err = OS_ERR_RDY_QUEUE_FULL;
        return;
    }

    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();
    OSEdfHeap[OSEdfHeapSize++] = p_tcb;
    p_tcb->EDFHeapIndex = OSEdfHeapSize - 1;
    *p_err = OS_ERR_NONE;

    OS_EdfHeapSiftUp(OSEdfHeapSize - 1);
    OS_CRITICAL_EXIT();
}

void OS_EdfHeapCheckHelper(uint32_t idx)
{
    uint32_t left_idx = 2 * idx + 1;
    uint32_t right_idx = left_idx + 1;

    ASSERT(OSEdfHeap[idx]->EDFHeapIndex == idx);
    if (left_idx < OSEdfHeapSize)
    {
        ASSERT(EDF_DEADLINE_ABSOLUTE(OSEdfHeap[left_idx]) >= EDF_DEADLINE_ABSOLUTE(OSEdfHeap[idx]));
        OS_EdfHeapCheckHelper(left_idx);
    }
    if (right_idx < OSEdfHeapSize)
    {
        ASSERT(EDF_DEADLINE_ABSOLUTE(OSEdfHeap[right_idx]) >= EDF_DEADLINE_ABSOLUTE(OSEdfHeap[idx]));
        OS_EdfHeapCheckHelper(right_idx);
    }
}

void OS_EdfHeapCheck()
{
    OS_EdfHeapCheckHelper(0);
}

void OS_EdfHeapRemove(OS_TCB* p_tcb)
{
    // for efficiency we should probably keep a reverse mapping.
    // simple implementation first though.
    CPU_SR_ALLOC();

    OS_CRITICAL_ENTER();
    uint32_t idx = p_tcb->EDFHeapIndex;

    OS_TCB* last_task = OSEdfHeap[--OSEdfHeapSize];
    OSEdfHeap[idx] = last_task;
    last_task->EDFHeapIndex = idx;
    int parent_idx = (idx - 1) / 2;
    /* Sift down if greater than or equal to parent, up otherwise. */
    if (OS_EdfHeapCompare(last_task, OSEdfHeap[parent_idx]) >= 0)
        OS_EdfHeapSiftDown(idx);
    else
        OS_EdfHeapSiftUp(idx);

#if EDF_CFG_HEAP_CHECK_VALID_EN
    OS_EdfHeapCheck();
#endif
    OS_CRITICAL_EXIT();
}
#endif
