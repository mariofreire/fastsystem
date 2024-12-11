[bits 16]
global putc

putc:
mov ah, 0x0e
int 0x10
ret
