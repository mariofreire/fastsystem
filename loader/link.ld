ENTRY(start)
SECTIONS
{
  . = 0xa000;
  .text : 
  {
	code = .;
	*(EXCLUDE_FILE (pci.o) .text);
	*(EXCLUDE_FILE (ahci.o) .text);
	*(EXCLUDE_FILE (ehci.o) .text);
	*(EXCLUDE_FILE (pci.o) .rodata*);
	*(EXCLUDE_FILE (ahci.o) .rodata*);
	*(EXCLUDE_FILE (ehci.o) .rodata*);
  }
  .data : 
  {
    data = .;
	*(EXCLUDE_FILE (pci.o) .data);
	*(EXCLUDE_FILE (ahci.o) .data);
	*(EXCLUDE_FILE (ehci.o) .data);
  }
  .bss : 
  {
    bss = .;
    *(EXCLUDE_FILE (pci.o) .bss*);
    *(EXCLUDE_FILE (ahci.o) .bss*);
    *(EXCLUDE_FILE (ehci.o) .bss*);
  }
  .enhanced_code : 
  {
	enhanced_code = .;
	pci.o(.enhanced_code);
	ahci.o(.enhanced_code);
	ehci.o(.enhanced_code);
  }
  .enhanced_data : 
  {
	enhanced_data = .;
	pci.o(.enhanced_data);
	ahci.o(.enhanced_data);
	ehci.o(.enhanced_data);
  }
  end = .;
}
