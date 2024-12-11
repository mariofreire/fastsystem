[org 0x7c00]
[bits 16]

fat equ 0x7e00
main_cat equ 0x9000
free_mem equ 0xac00
kernel_offs equ 0x1000
boot_drive equ 0x00
fat32_type equ 0x0c

boot:
jmp start
nop

oem_name db 'FASTSYS1'
fat_info times 71-($-$$) db 0
volume_label db 'NEW DISK   '
fat_type db 'FAT32   '

start:
mov ax, 0
xor ax, ax
mov ss, ax
mov sp, 0x7c00
sti
push ax
pop es
push ax
pop ds
cld
mov dl, 0x80
mov [boot_drive], dl
mov bx, MsgLoad
call print

codestart:

;mov ax, word [bx+8]
;mov bx, 512
;imul bx
;xchg ax, dx
;mov si, ax
mov cl, [bx+8]
mov ch, 0
; load FAT
pusha
push dx
mov al, 1
mov ah, 2
mov dl, 0x80
mov dh, 1
mov ch, 0
mov cl, 1
mov bx, fat
int 0x13
pop dx
popa
mov bx, fat
mov dx, word [bx]
call print_hex
;mov ax, fat
;call word ax
hlt
jmp $


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


; receiving the data in 'dx'
; For the examples we'll assume that we're called with dx=0x1234
print_hex:
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

filename db 'LOADER     ',0
MsgLoad db  'Starting FastSystem...',13,10,0
msg db 'LOADER not found.',0

times 510-($-$$) db 0
dw 0xaa55

