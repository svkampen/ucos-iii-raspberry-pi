#pragma once

/* Power-of-two minus one, please. */
#define EDF_CFG_MAX_TASKS 511
#define EDF_CFG_CHECK_DEADLINE_MISS 1u
#define EDF_CFG_ENABLED 1u
#define EDF_CFG_DEBUG 0u

#define EDF_DEADLINE_ABSOLUTE(tcb)                                             \
    ((tcb)->CurrentActivationTime + (tcb)->EDFRelativeDeadline)

#define OS_EDF_HEAP_FOREACH(body)                                              \
    for (int32_t idx = 0; idx < OSEdfHeapSize; ++idx)                         \
    {                                                                          \
        OS_TCB* task = OSEdfHeap[idx];                                         \
        body                                                                   \
    }
