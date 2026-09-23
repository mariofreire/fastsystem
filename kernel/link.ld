ENTRY(start)
kernel_start = .;
SECTIONS
{
    . = 0xC000000;
 
    .text ALIGN (0x1000) :
    {
		*(.multiboot)
        *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)     
    }

    .bss ALIGN (0x1000) :  
    {
        *(COMMON)         
        *(.bss)       
    }
	
    kernel_end = .; _end = .; __end = .;
}
