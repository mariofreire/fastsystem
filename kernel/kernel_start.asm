[bits 32]

section .multiboot
	align 8
	magic:
        dd 0x1BADB002
	flags:
        dd 0x03
	checksum:
        dd - (0x1BADB002 + 0x03)

section .text

global start
[extern main]
[extern kernel_end]

KERNEL_STACK_SIZE equ 2048

start:
cli
xor eax, eax
mov ax, 0x10
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov gs, ax
mov esp, 0xF000000
;mov esp, kstack_top
;mov esp, kernel_end
;add esp, KERNEL_STACK_SIZE
xor ebp, ebp
push ebx
call 0x08:main
;mov esp, 0xC000000
;call 0x08:main
hlt
jmp $

print32:
mov ebx, 0xb8000
print32_loop:
lodsb
or al, al
jz print32_done
or eax, 0x0700
mov word [ebx], ax
add ebx, 2
jmp print32_loop
print32_done:
ret

section .bss
	resb 16384
;align 16
;kstack: resb 16384
;kstack: resb 2048
;kstack_top:
;global kstack_top

section .note.GNU-stack
