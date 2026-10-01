org 0x7C00
bits 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    xor ax, ax

    mov si, msg_invalid
    
print_string:
    lodsb
    cmp al, 0
    je halt_system
    
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print_string

halt_system:
    cli
    hlt
    jmp halt_system

msg_invalid db 'Invalid partition table', 0x0D, 0x0A, 0
msg_loading db 'Error loading operating system', 0x0D, 0x0A, 0
msg_missing db 'Missing operating system', 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55
