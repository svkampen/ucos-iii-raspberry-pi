#include "atag.h"
#include "io.h"
#include "uart.h"

/* ATAG code included from https://github.com/mrvn/RaspberryPi-baremetal/blob/master/004-a-t-a-and-g-walk-into-a-baremetal/arch_info.c
 * Licensed under the GPL, as are the rest of my modifications. */

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

const struct ATag* next(const struct ATag* tag)
{
    if (tag->tag == ATAG_NONE)
    {
        return NULL;
    }

    return (const struct ATag*)(((uint32_t*)tag) + tag->tag_size);
}

u32 arch_info_init(const struct ATag* tag)
{
    u32 memory_size = 0;
    do
    {
        switch (tag->tag)
        {
            case ATAG_MEM:
                uart_send("Found memory tag: size ");
                printi(tag->mem_size);
                uart_send(", start ");
                printi(tag->mem_start);
                memory_size += tag->mem_size - tag->mem_start;
                break;
            case ATAG_CMDLINE:
                uart_send("Found command line: ");
                uart_send(tag->cmdline);
                uart_send("\n");
                break;
            case ATAG_CORE:
                uart_send("Found core tag.\n"); break;
            case ATAG_INITRD2:
                uart_send("Found initrd tag.\n"); break;
            default:
                uart_send("Found other tag: ");
                printi(tag->tag);
                uart_send("\n");
        }
    } while ((tag = next(tag)));

    return memory_size;
}
