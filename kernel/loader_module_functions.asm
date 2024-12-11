[bits 32]

section .text
global probememory


probememory:
	cli
	mov ecx, 0x001FFFFC
countmemory:
	mov dword [gs:ecx], 0x55AA55AA
	cmp dword [gs:ecx], 0x55AA55AA
	jne limitreached
	mov dword [gs:ecx], 0xAA55AA55
	cmp dword [gs:ecx], 0xAA55AA55
	jne limitreached
	add ecx, 0x00100000
	cmp ecx, 0xFFFFFFFC
	jne countmemory
limitreached:	
	mov eax, ecx
	sub eax, 0x000FFFFC
	sti
	ret

