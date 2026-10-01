ENTRY(start)
kernel_start = .;
SECTIONS
{
    . = 0xC000000;
 
    .text :
    {
		*(.multiboot)
    	*(.text)
    	*(.text*)
    }

    .rodata : ALIGN(4)
    {
        *(.rodata)
    	*(.rodata*)
    }

    .data : ALIGN(4)
    {
        *(.data)     
    	*(.data*)
    }

    .bss : ALIGN(4)
    {
        *(COMMON)
        *(.bss)    
    	*(.bss*)   
    }
	
    kernel_end = .; _end = .; __end = .;
}
