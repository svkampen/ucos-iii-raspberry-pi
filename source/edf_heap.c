#include <edf_cfg.h>
#include <includes.h>
#include <printf.h>

#if EDF_CFG_ENABLED
OS_TCB*    OSEdfHeap[EDF_CFG_MAX_TASKS];
CPU_INT32U OSEdfHeapSize;

int OS_EdfHeapCompare(OS_TCB* a, OS_TCB* b)
{
    CPU_TS aAbsDeadline = a->EDFLastActivationTime + a->EDFRelativeDeadline;
    CPU_TS bAbsDeadline = b->EDFLastActivationTime + a->EDFRelativeDeadline;
    return (aAbsDeadline > bAbsDeadline) ? 1 : ((aAbsDeadline < bAbsDeadline) ? -1 : 0);
}

OS_TCB* OS_EdfHeapPeek(void)
{
    return OSEdfHeap[0];
}

void OS_EdfHeapSiftUp(int index)
{
    int   compare_result, parent_idx;
    void* tmp;

    if (index == 0) return;

    parent_idx     = (index - 1) / 2;
    compare_result = OS_EdfHeapCompare(OSEdfHeap[index], OSEdfHeap[parent_idx]);

    while (compare_result < 0)
    {
        tmp              = OSEdfHeap[index];
        OSEdfHeap[index]      = OSEdfHeap[parent_idx];
        OSEdfHeap[parent_idx] = tmp;

        index = parent_idx;

        parent_idx     = (index - 1) / 2;
        compare_result = OS_EdfHeapCompare(OSEdfHeap[index], OSEdfHeap[parent_idx]);
    }
}

void OS_EdfHeapSiftDown(int index)
{
    int   left_index, right_index;
    void *element, *left_element, *right_element;
    int   l_or_r_swap;

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
                OSEdfHeap[index]      = left_element;
                index            = left_index;
            }
            else
            {
                OSEdfHeap[right_index] = element;
                OSEdfHeap[index]       = right_element;
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
    for (int i = 0; i < EDF_CFG_MAX_TASKS; ++i)
    {
        OSEdfHeap[i] = NULL;
    }
}

void OS_EdfHeapInsert(OS_TCB* p_tcb, OS_ERR* p_err)
{
    if (OSEdfHeapSize == EDF_CFG_MAX_TASKS)
    {
        *p_err = OS_ERR_RDY_QUEUE_FULL;
        return;
    }
    OSEdfHeap[OSEdfHeapSize++] = p_tcb;
    *p_err = OS_ERR_NONE;

    OS_EdfHeapSiftUp(OSEdfHeapSize - 1);
}

void OS_EdfHeapRemove(OS_TCB* p_tcb)
{
    // for efficiency we should probably keep a reverse mapping.
    // simple implementation first though.
    int idx = -1;
    for (int i = 0; i < OSEdfHeapSize; ++i)
    {
        if (OSEdfHeap[i] == p_tcb)
            idx = i;
            break;
    }

    if (idx == -1)
        printf("Trying to remove a task from heap that's not on the heap?");

    OS_TCB* last_task = OSEdfHeap[OSEdfHeapSize--];
    OSEdfHeap[idx] = last_task;
    int parent_idx = (idx - 1) / 2;
    /* Sift down if greater than or equal to parent, up otherwise. */
    if (OS_EdfHeapCompare(last_task, OSEdfHeap[parent_idx]) > 0)
        OS_EdfHeapSiftDown(idx);
    else
        OS_EdfHeapSiftUp(idx);

#if EDF_CFG_HEAP_CHECK_VALID_EN
    OS_EdfHeapCheck();
#endif

}
#endif
