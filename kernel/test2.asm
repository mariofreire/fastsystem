[bits 32]

section .text
global _start

	_start:
		mov esi, eax         ; initial offset
		
		mov eax, 4           ; syscall_write
		mov ebx, 1           ; fd = stdout (1)
		mov ecx, msg1        ; buffer pointer
		add ecx, esi         ; set offset to begin
		mov edx, msg1len     ; string length
		int 0x80 ; syscall

		mov eax, 1           ; syscall_exit
		mov ebx, 0           ; arg1 x - exit(x)
		int 0x80             ; syscall

section .data
	msg1 db "Nova Mensagem.", 10
	msg1len equ $-msg1
