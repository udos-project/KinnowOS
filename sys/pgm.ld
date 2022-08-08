ENTRY(_start);

SECTIONS {
    . = 8M;
    
    .text : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(.init .init.*);
        *(.fini .fini.*);
        *(.text .text.*);
        *(.stext .stext.*);
    }
    
    .data : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(.data .data.*);
        *(.sdata .sdata.*);
        *(.eh_frame .eh_frame_hdr);
    }

    .rodata : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(.rodata .rodata.*);
        *(.srodata .srodata.*);
    }

    .bss : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(COMMON);
        *(.bss .bss.*);
        *(.sbss .sbss.*);
    }

    .note : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(.note .note.*);
    }
}
