#pragma once
#include "types.h"

struct atag {
    uint32_t tag_size;
    uint32_t tag;
    union
    {
        struct {
            uint32_t mem_size;
            uint32_t mem_start;
        };
        struct {
            uint32_t initrd_start;
            uint32_t initrd_size;
        };
        struct {
            char cmdline[1];
        };
    };
};

u32 arch_info_init(const struct atag* tag);
