#include "atag.h"
#include "printf.h"
#include "uart.h"

#include <stddef.h>

#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009

const struct atag* next(const struct atag* tag)
{
    if (tag->tag == ATAG_NONE)
    {
        return NULL;
    }

    return (const struct atag*)(((uint32_t*)tag) + tag->tag_size);
}

u32 arch_info_init(const struct atag* tag)
{
    u32 memory_size = 0;
    do
    {
        switch (tag->tag)
        {
            case ATAG_MEM:
                printf("Found memory tag: size %lu, start %lu\n",
                       tag->mem_size,
                       tag->mem_start);
                memory_size += tag->mem_size - tag->mem_start;
                break;
            case ATAG_CMDLINE:
                printf("Found command line: %s\n", tag->cmdline);
                break;
            case ATAG_CORE:
                uart_send("Found core tag.\n"); break;
            case ATAG_INITRD2:
                uart_send("Found initrd tag.\n"); break;
            default:
                printf("Found other tag: %lx\n", tag->tag);
                break;
        }
    } while ((tag = next(tag)));

    return memory_size;
}
