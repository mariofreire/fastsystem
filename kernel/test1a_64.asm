[bits 64]

section .text
global _start

	_start:
		mov rsi, rax         ; set offset to begin
		add rsi, msg1        ; buffer pointer
		mov rax, 1           ; syscall_write
		mov rdi, 1           ; fd = stdout (1)
		mov rdx, msg1len     ; string length
		syscall

		mov rax, 60          ; syscall_exit
		mov rdi, 0           ; arg1 x - exit(x)
		syscall

section .data
	msg1 db "Hello World!", 10
	msg1len equ $-msg1
