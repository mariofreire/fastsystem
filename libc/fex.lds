OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

INPUT(libc.a)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS {
	/* 1. Fast System Executable and Linkable Format (FEXLF) Header Subsystem */
	. = 0x100000; /* Target load address */
	
	.fexlf_header : AT(0x1000) {
    	QUAD(0x464C584546)  /* Magic Number: "FEXLF" in hex */
    	LONG(_start)        /* Entry point address */
    	LONG(_fexlf_end)    /* Total size indicator */
    	SHORT(0x0004)       /* Subsystem ID (e.g., 3 for Shared, 4 for Executable) */
	}
	
	_seg_start = .;
	
	/* 2. Segments */
	.text : {
    	*(.text)
    	*(.text*)
	} :text
	
	.rodata (READONLY) : {
    	*(.rodata)
    	*(.rodata*)
	} :text
	
	.data : {
    	*(.data)
    	*(.data*)
	} :data
	
	.bss : {
    	*(.bss)
    	*(.bss*)
	} :data
	
	_seg_end = .;
}
