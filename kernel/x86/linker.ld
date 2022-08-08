ENTRY(start);
MEMORY {
	ram (rwx) : ORIGIN = 0x100000, LENGTH = 4096M
}

PHDRS {
	null PT_NULL FLAGS(0);
	text PT_LOAD FLAGS((1 << 0) | (1 << 2));
	rodata PT_LOAD FLAGS((1 << 2));
	data PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS {
	kernel_start = .;

	head_start = .;
	.head : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.multiboot);
		*(.head.*);
	} >ram :text
	head_end = .;

	text_start = .;
	.text : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.text .text.*);
	} >ram :text
	text_end = .;

	data_start = .;
	.data : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.data .data.*);
		*(.sdata .sdata.*);
		*(.eh_frame .eh_frame_hdr);
	} >ram :data
	data_end = .;

	rodata_start = .;
	.rodata : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.rodata .rodata.*);
		*(.srodata .srodata.*);
	} >ram :rodata
	rodata_end = .;

	bss_start = .;
	.bss : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(COMMON);
		*(.bss .bss.*);
		*(.sbss .sbss.*);
	} >ram :data
	bss_end = .;
	
	.note : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.note .note.*);
	} >ram :data

	. = ALIGN(4K);
	heap_start = .;
}

