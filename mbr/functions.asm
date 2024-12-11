[bits 16]
global putc
global print
global print_hex
global getdrivenumber
global oserror
global diskread

getdrivenumber:
	mov al, dl
	ret

putc:
	mov ah, 0x0e
	int 0x10
	ret

print:
	pusha
print_next:
	mov al, [bx]
	cmp al, 0
	je print_end
	mov ah, 0x0e
	int 0x10
	add bx, 1
	jmp print_next
print_end:
	popa
	ret

print_hex:
	mov bx, sp
	pusha
	mov cx, 0
	mov ax, word [bx+4]
	mov dx, ax
	call print_hex_data
	popa
	ret

; receiving the data in 'dx'
; For the examples we'll assume that we're called with dx=0x1234
print_hex_data:
    pusha

    mov cx, 0 ; our index variable

; Strategy: get the last char of 'dx', then convert to ASCII
; Numeric ASCII values: '0' (ASCII 0x30) to '9' (0x39), so just add 0x30 to byte N.
; For alphabetic characters A-F: 'A' (ASCII 0x41) to 'F' (0x46) we'll add 0x40
; Then, move the ASCII byte to the correct position on the resulting string
hex_loop:
    cmp cx, 4 ; loop 4 times
    je end
    
    ; 1. convert last char of 'dx' to ascii
    mov ax, dx ; we will use 'ax' as our working register
    and ax, 0x000f ; 0x1234 -> 0x0004 by masking first three to zeros
    add al, 0x30 ; add 0x30 to N to convert it to ASCII "N"
    cmp al, 0x39 ; if > 9, add extra 8 to represent 'A' to 'F'
    jle step2
    add al, 7 ; 'A' is ASCII 65 instead of 58, so 65-58=7

step2:
    ; 2. get the correct position of the string to place our ASCII char
    ; bx <- base address + string length - index of char
    mov bx, HEX_OUT + 5 ; base + length
    sub bx, cx  ; our index variable
    mov [bx], al ; copy the ASCII char on 'al' to the position pointed by 'bx'
    ror dx, 4 ; 0x1234 -> 0x4123 -> 0x3412 -> 0x2341 -> 0x1234

    ; increment index and loop
    add cx, 1
    jmp hex_loop

end:
    ; prepare the parameter and call the function
    ; remember that print receives parameters in 'bx'
    mov bx, HEX_OUT
    call print

    popa
    ret




HEX_OUT:
    db '0x0000',0 ; reserve memory for our new string


DAPACK:
	db	0x10
	db	0
blkcnt:	dw	16		; int 13 resets this to # of blocks actually read/written
db_add:	dw	0x7C00		; memory buffer destination address (0:7c00)
	dw	0		; in memory page zero
d_lba:	dd	1		; put the lba to read in this spot
	dd	0		; more storage bytes only for big lba's ( > 4 bytes )

diskread:
	mov bx, sp
	pusha
	mov cx, 0
	mov ax, word [bx+4]
	mov word [db_add], ax
	mov ax, word [bx+8]
	mov word [d_lba], ax
	popa
	mov si, DAPACK		; address of "disk address packet"
	mov ah, 0x42		; AL is unused
	mov dl, 0x80		; drive number 0 (OR the drive # with 0x80)
	int 0x13
	jc diskerror
	jnc diskok
	ret

diskerror:
	mov ax, 0
	ret

diskok:
	mov ax, 1
	ret
	
oserror:
	mov bx, osmsgerr
	call print
	ret

osmsgerr:  db 'Missing operating system.',0
	