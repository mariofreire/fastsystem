[bits 32]

section .text
global _start

	_start:
		mov esi, eax         ; initial offset
		
    ; ssize_t read(int fd, void *buf, size_t count)
	
    	mov eax, 3          ; sys_read
    	mov ebx, 0          ; fd = stdin
    	mov ecx, buf        ; buffer
    	mov edx, buflen     ; maximum bytes
    	int 0x80
	
    	; eax = number of bytes actually read
	    mov edx, eax
	    
	    mov ecx, buf
	    mov eax, 4          ; sys_write
	    mov ebx, 1          ; fd = stdout
	    int 0x80
	    	    
    	mov eax, 1          ; sys_exit
    	xor ebx, ebx
    	int 0x80
	
section .data
	buf times 100 db 0
	buflen equ $-buf
	
