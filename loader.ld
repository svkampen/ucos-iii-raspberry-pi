SECTIONS {
    .init 0x8000 : {
        *(.init)
    }

    __init_end = .;

    .text 0x6400000 : AT(ADDR(.init) + SIZEOF(.init)) {
        *(.text)
        *(.text.*)
    }

    .data : {
        *(.data*)
        *(.rodata*)
    }

    __data_end = .;

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }


    /DISCARD/ : {
        *(*)
    }
}
