PHDRS {
	null PT_NULL FLAGS(0);
	text PT_LOAD FLAGS((1 << 0) | (1 << 2));
	rodata PT_LOAD FLAGS((1 << 2));
	data PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS {
	kernel_start = .;

	head_start = .;
	.head : ALIGN(CONSTANT(PAGE_SIZE)) {
		*(.head.*);
	} >ram :data
	head_end = .;

	rodata_start = .;
	.rodata : ALIGN(CONSTANT(PAGE_SIZE)) {
		*(.rodata .rodata.*);
		*(.srodata .srodata.*);
	} >ram :rodata
	rodata_end = .;

	text_start = .;
	.text : ALIGN(CONSTANT(PAGE_SIZE)) {
		*(.text .text.*);
	} >ram :text
	text_end = .;

	data_start = .;
	.data : ALIGN(CONSTANT(PAGE_SIZE)) {
		*(.data .data.*);
		*(.sdata .sdata.*);
		*(.eh_frame .eh_frame_hdr);
	} >ram :data
	data_end = .;

	bss_start = .;
	.bss : ALIGN(CONSTANT(PAGE_SIZE)) {
		*(COMMON);
		*(.bss .bss.*);
		*(.sbss .sbss.*);
	} >ram :data
	bss_end = .;
	
	.note : ALIGN(CONSTANT(PAGE_SIZE)) {
		*(.note .note.*);
	} >ram :data

	. = ALIGN(4K);
	heap_start = .;
}
