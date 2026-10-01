OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

INPUT(libc.a)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS 
{
	. = 0x100000;
	
	_seg_start = .;
	
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
