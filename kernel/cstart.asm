[bits 32]

section .text
global _start
extern main

_start:
	push ebp
	mov ebp, esp
	push ebx
	sub esp, 1056
	mov dword [ebp-16], 0
	mov dword [ebp-8], 0
	mov eax, edi
	mov dword [ebp-20], eax
	mov eax, esi
	mov dword [ebp-24], eax
	mov dword [ebp-12], 0
	jmp _get_argv_string_main
_add_argv_str_n_main:
	mov edx, dword [ebp-24]
	mov eax, dword [ebp-8]
	add edx, eax
	mov eax, dword [ebp-12]
	mov dword [ebp-1056+eax*4], edx
	add dword [ebp-8], 256
	inc dword [ebp-12]
_get_argv_string_main:
	cmp dword [ebp-12], 255
	jle _add_argv_str_n_main
	lea eax, [ebp-1056]
	mov dword [ebp-28], eax
	mov eax, dword [ebp-28]
	mov dword [ebp-32], eax
	push dword [ebp-32]
	push dword [ebp-20]
	call main
	add esp, 8
	mov dword [ebp-16], eax
	mov eax, 1
	mov edx, dword [ebp-16]
	mov ebx, edx
	int 0x80
	
section .bss
	resb 8192
