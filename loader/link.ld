ENTRY(start)
SECTIONS
{
  . = 0xa000;
  .text : 
  {
	code = .;
	*(EXCLUDE_FILE (pci.o) .text);
	*(EXCLUDE_FILE (pci.o) .rodata*);
  }
  .data : 
  {
    data = .;
	*(EXCLUDE_FILE (pci.o) .data);
  }
  .bss : 
  {
    bss = .;
    *(EXCLUDE_FILE (pci.o) .bss*);
  }
  .enhanced_code : 
  {
	enhanced_code = .;
	pci.o(.enhanced_code);
  }
  .enhanced_data : 
  {
	enhanced_data = .;
	pci.o(.enhanced_data);
  }
  end = .;
}
