#pragma once
#include <inttypes.h>

#define u8 uint8_t
#define u32 uint32_t
#define vu8 volatile uint8_t
#define vu32 volatile uint32_t

extern void put32(u32 addr, u32 val);
extern u32 get32(u32 addr);
extern void wait_cycles(u32 cycles);
extern void __dmb(void);
