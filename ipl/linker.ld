ENTRY(start);

SECTIONS {
    . = 0;
    .init : {
        KEEP(*(.rodata.init*));
    }

    /* There is an area between 0x200 and 0x1000 are not reserved - this also applies
     * to both z/Arch, S/360, S/370 and S/390 */
    . = 0x200;

    .text : {
        KEEP(*(.text.start));
        *(.text*);
        *(.rodata*);
        *(.data*);
        *(.bss*);
    }
    
    disk_buffer = 0x2000;
}
