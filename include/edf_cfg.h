#pragma once

/* Power-of-two minus one, please. */
#define EDF_CFG_MAX_TASKS 255u
#define EDF_CFG_CHECK_DEADLINE_MISS 1u
#define EDF_DEADLINE_ABSOLUTE(tcb) ((tcb)->EDFLastActivationTime + (tcb)->EDFRelativeDeadline)
#define EDF_CFG_ENABLED 1u

#define TICKS_TO_USEC(ticks) ((1000000 * ticks) / OSCfg_TickRate_Hz)
