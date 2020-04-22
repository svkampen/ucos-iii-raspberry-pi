SECTIONS {
    .init 0x8000 : {
        *(.init)
    }

    .text : {
        *(.text)
        *(.text.*)
    }

    .data : {
        *(.data*)
        *(.rodata*)
    }

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    __kernel_top = .;

    /DISCARD/ : {
        *(*)
    }
}
