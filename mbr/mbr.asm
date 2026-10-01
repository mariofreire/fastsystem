bits 16
org 0x7C00

start:
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00
    mov es, ax
    mov ds, ax
    mov si, 0x7C00
    mov di, 0x0600
    mov cx, 0x0200
    cld
    rep movsb

    push ax
    push word 0x061C
    retf

relocated:
    sti
    mov cx, 0x0004
    mov bp, 0x07BE

partition_loop:
    cmp byte [bp], 0x00
    jl active_partition
    jnz invalid_partition

    add bp, 0x10
    loop partition_loop

    int 0x18

active_partition:
    mov [bp], dl
    push bp
    mov byte [bp + 0x11], 0x05
    mov byte [bp + 0x10], 0x00

    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13

    pop bp
    jc no_extensions

    cmp bx, 0xAA55
    jnz no_extensions

    test cx, 0x0001
    jz no_extensions

    inc byte [bp + 0x10]

no_extensions:
    pushad

    cmp byte [bp + 0x10], 0x00
    jz chs_read

    push dword 0x00000000
    push dword [bp + 0x08]
    push word 0x0000
    push word 0x7C00
    push word 0x0001
    push word 0x0010

    mov ah, 0x42
    mov dl, [bp]
    mov si, sp
    int 0x13

    lahf
    add sp, 0x0010
    sahf

    jmp short read_result

chs_read:
    mov ax, 0x0201
    mov bx, 0x7C00
    mov dl, [bp]
    mov dh, [bp + 0x01]
    mov cl, [bp + 0x02]
    mov ch, [bp + 0x03]
    int 0x13

read_result:
    popad
    jnb read_success

    dec byte [bp + 0x11]
    jnz retry_disk

    cmp byte [bp], 0x80
    jz loading_error

    mov dl, 0x80
    jmp short active_partition

retry_disk:
    push bp
    xor ah, ah
    mov dl, [bp]
    int 0x13
    pop bp
    jmp short no_extensions

read_success:
    cmp word [0x7DFE], 0xAA55
    jnz missing_os

    push word [bp]

    call a20_check

    jnz tpm_check

    cli
    mov al, 0xD1
    out 0x64, al

    call a20_check

    mov al, 0xDF
    out 0x60, al

    call a20_check

    mov al, 0xFF
    out 0x64, al

    call a20_check

    sti

tpm_check:
    mov ax, 0xBB00
    int 0x1A

    and eax, eax
    jnz boot_partition

    cmp ebx, 0x41504354
    jnz boot_partition

    cmp cx, 0x0102
    jb boot_partition

    push dword 0x0000BB07
    push dword 0x00000200
    push dword 0x00000008
    pushad

    push dword 0x00000000
    push dword 0x00007C00

    popad

    push word 0x0000
    pop es

    int 0x1A

boot_partition:
    pop dx
    xor dh, dh
    jmp 0x0000:0x7C00

    int 0x18

missing_os:
    mov al, [0x07B7]
    jmp short print_message

loading_error:
    mov al, [0x07B6]
    jmp short print_message

invalid_partition:
    mov al, [0x07B5]

print_message:
    xor ah, ah
    add ax, 0x0700
    mov si, ax

print_loop:
    lodsb
    cmp al, 0x00
    jz halt_system

    mov bx, 0x0007
    mov ah, 0x0E
    int 0x10
    jmp short print_loop

halt_system:
    hlt
    jmp short halt_system

a20_check:
    xor cx, cx

a20_wait:
    in al, 0x64
    jmp short a20_test

a20_test:
    and al, 0x02
    loopne a20_wait

    and al, 0x02
    ret

msg_invalid:
    db 'Invalid partition table', 0x00

msg_loading:
    db 'Error loading operating system', 0x00

msg_missing:
    db 'Missing operating system', 0x00

times 0x1B5 - ($ - $$) db 0x00

db 0x00
db 0x00
db 0x00

times 510 - ($ - $$) db 0x00

dw 0xAA55
