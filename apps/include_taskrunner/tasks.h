#include <stdint.h>
#include <os.h>

struct task
{
    uint32_t rm_priority;
    uint64_t edf_period;
    uint64_t edf_relative_deadline;
    uint64_t wcet;
    OS_TCB tcb __attribute__((aligned(8)));
    CPU_STK stk[2048] __attribute__((aligned(8)));
};

struct task_set
{
    uint32_t num_tasks;
    uint64_t hyperperiod;
    struct task tasks[];
};

extern struct task_set task_set;
