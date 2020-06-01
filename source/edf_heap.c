#include <edf_cfg.h>
#include <includes.h>
#include <printf.h>
#include <runtime_assert.h>

#if EDF_CFG_ENABLED
int OS_EdfHeapCompare(OS_TCB* a, OS_TCB* b)
{
    CPU_TS64 aAbsDeadline = EDF_DEADLINE_ABSOLUTE(a);
    CPU_TS64 bAbsDeadline = EDF_DEADLINE_ABSOLUTE(b);
    return (aAbsDeadline > bAbsDeadline) ? 1 : ((aAbsDeadline < bAbsDeadline) ? -1 : 0);
}

OS_TCB* OS_EdfHeapPeek(void)
{
    ASSERT(OSEdfHeapSize != 0);
    return OSEdfHeap[0];
}

void print_task_name_deadline(int idx, OS_TCB* task)
{
    printf("%d %s P %llx RD %llx LAT %llx", idx, task->NamePtr, task->EDFPeriod, task->EDFRelativeDeadline, task->EDFCurrentActivationTime);
}

void OS_EdfResetActivationTimes()
{
    uint64_t tmr = CPU_TS_TmrRd();
    OS_EDF_HEAP_FOREACH({
        task->EDFCurrentActivationTime = tmr;
    });
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

void print_helper(int32_t idx)
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
    int32_t left_index, right_index;
    OS_TCB *element, *left_element, *right_element;
    int l_or_r_swap;

    if (index < 0) ASSERT(false);

#if EDF_CFG_DEBUG
    printf("Heap before sift-down: ");
    OS_EdfPrintHeap();
#endif

    while (DEF_ON)
    {
        left_index  = 2 * index + 1;
        right_index = left_index + 1;

        if (left_index < 0 || right_index < 0) ASSERT(false);

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
    for (int32_t i = 0; i < EDF_CFG_MAX_TASKS; ++i)
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

    int32_t idx = p_tcb->EDFHeapIndex;

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
        *p_err = OS_ERR_EDF_RDY_QUEUE_FULL;
        return;
    }

    // already in heap.
    if (p_tcb->EDFHeapIndex != -1)
    {
#if EDF_CFG_DEBUG
        printf("Not inserting task at index %ld: `%s' = `%s'\n",
                p_tcb->EDFHeapIndex, p_tcb->NamePtr, OSEdfHeap[p_tcb->EDFHeapIndex]->NamePtr);
#endif
        return;
    }

    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();

#if EDF_CFG_DEBUG
    printf("EdfHeapInsert `%s'\n", p_tcb->NamePtr);
#endif

    OSEdfHeap[OSEdfHeapSize++] = p_tcb;
    p_tcb->EDFHeapIndex = OSEdfHeapSize - 1;
    *p_err = OS_ERR_NONE;

    OS_EdfHeapSiftUp(OSEdfHeapSize - 1);

#if EDF_CFG_DEBUG
    printf("Heap after insert: ");
    OS_EdfPrintHeap();
#endif
    OS_CRITICAL_EXIT();
}

static void OS_EdfHeapCheckHelper(int32_t idx)
{
    if (!(idx < OSEdfHeapSize)) return;
    int32_t left_idx = 2 * idx + 1;
    int32_t right_idx = left_idx + 1;

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
    uint32_t lr_addr;
    __asm__("mov %0,lr" : "=r"(lr_addr));

    CPU_SR_ALLOC();

    OS_CRITICAL_ENTER();
#if EDF_CFG_DEBUG
    printf("OSEdfHeapRemove `%s' called from %08lx; new heap size %ld\n", p_tcb->NamePtr, lr_addr, OSEdfHeapSize - 1);
#endif
    int32_t idx = p_tcb->EDFHeapIndex;
    ASSERT(idx != -1);
    p_tcb->EDFHeapIndex = -1;

    OS_TCB* last_task = OSEdfHeap[--OSEdfHeapSize];
    OSEdfHeap[OSEdfHeapSize] = NULL;
    if (last_task == p_tcb) goto end;
    OSEdfHeap[idx] = last_task;
    last_task->EDFHeapIndex = idx;
    int parent_idx = (idx - 1) / 2;
    /* Sift down if greater than or equal to parent, up otherwise. */
    if (OS_EdfHeapCompare(last_task, OSEdfHeap[parent_idx]) >= 0)
    {
#if EDF_CFG_DEBUG
        printf("Sifting down...\n");
#endif
        OS_EdfHeapSiftDown(idx);
    }
    else
    {
#if EDF_CFG_DEBUG
        printf("Sifting up...\n");
#endif
        OS_EdfHeapSiftUp(idx);
    }

end:
#if EDF_CFG_DEBUG
    printf("Heap after remove: ");
    OS_EdfPrintHeap();
#endif
    ASSERT(p_tcb->EDFHeapIndex == -1);

#if EDF_CFG_HEAP_CHECK_VALID_EN
    OS_EdfHeapCheck();
#endif
    OS_CRITICAL_EXIT();
}
#endif
