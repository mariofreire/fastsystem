SECTIONS
{
  . = 0x7c00;

  .cseg : AT(0x7c00)
  {
    *(.text);
  }

  .dseg : AT(0x7ce0)
  {
    *(.rodata);
  }

  .pts : AT(0x7db4)
  {
    BYTE(0x00);
	  . = ALIGN(16);
  }

  .sig : AT(0x7dfe)
  {
    SHORT(0xaa55);
  }

  /DISCARD/ :
  {
    *(.eh_frame);
  }
}