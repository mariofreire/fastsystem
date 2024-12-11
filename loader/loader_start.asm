section .text
[bits 16]

global start
[extern main]

start:
xor ax, ax
in al, 0x92
or al, 2
out 0x92, al
cli
mov sp, 0x5000
;mov sp, 0xf000
lgdt [gdt_pointer]
mov eax, cr0
or eax, 1
mov cr0, eax
mov cx, 1
jmp CODE_SEGMENT:loadkernel32
nop
jmp $

section .data
gdt:
    nop
gdt_start:
   dq 0x0
gdt_code:
   dw 0xFFFF
   dw 0x0
   db 0x0
   db 0x9A
   db 0xCF
   db 0x0
gdt_data:
   dw 0xFFFF
   dw 0x0
   db 0x0
   db 0x92
   db 0xCF
   db 0x0
gdt_end:
gdt_pointer:
   dw gdt_end - gdt_start
   dd gdt_start

CODE_SEGMENT equ gdt_code - gdt_start
DATA_SEGMENT equ gdt_data - gdt_start


[bits 32]

loadkernel32:
mov ax, DATA_SEGMENT
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov gs, ax
mov esp, 0xF00000
cli
mov al, 11111111b
out 0x21, al
in al, 0x70
or al, 10000000b
out 0x70, al
xor eax, eax
call main
hlt
jmp $

section .bss
	resb 16384
	
