#pragma once
#include "types.h"

#define GPIO_FN_INPUT 0
#define GPIO_FN_OUTPUT 1
/* broadcom logic incoming... */
#define GPIO_FN_ALT0 4
#define GPIO_FN_ALT1 5
#define GPIO_FN_ALT2 6
#define GPIO_FN_ALT3 7
#define GPIO_FN_ALT4 3
#define GPIO_FN_ALT5 2

#define GPIO_PUD_OFF 0
#define GPIO_PUD_UP 1
#define GPIO_PUD_DOWN 2

#define GPIO_LED_ON 0
#define GPIO_LED_OFF 1

void GPIO_SetFunction(u32 pin, u32 function);
void GPIO_Set(u32 pin, u32 value);
