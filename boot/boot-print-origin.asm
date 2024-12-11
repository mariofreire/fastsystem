[org 0x7c00]
[bits 16]

boot_drive equ 0x900

boot:
jmp start
nop

oem_name db 'FASTSYS1'
bytes_per_sector dw 0x0000
sector_per_cluster db  0x00
reserved_sectors_count dw  0x0000
number_fats db  0x00
root_entries_count dw  0x0000
total_sectors_16 dw  0x0000
media db  0x00
fat_size_16 dw  0x0000
sectors_per_track dw  0x0000
number_heads dw  0x0000
hidden_sectors dd  0x00000000
total_sectors_32 dd  0x00000000
fat_size_32 dd  0x00000000
flags dw  0x0000
version dw  0x0000
root_cluster dd  0x00000000
fs_info dw  0x0000
backup_boot_sector dw  0x0000
reserved_0 db 12 dup (0)
drive_number db  0x00
reserved_1 db  0x00
boot_signature db  0x00
volume_id dd  0x00000000
volume_label db 'NEW DISK   '
fat_type db 'FAT32   '

start:
xor ax, ax
mov [boot_drive], dl
xor ax, ax

codestart:
mov al, [boot_drive]
shr al, 7
shl al, 1
add al, 'A'
mov [bootDrv], al
xor ax, ax
call loadmsg
hlt
jmp $

loadmsg:
mov bx, bootMsg
call print
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


bootMsg db  'Booting from Drive '
bootDrv db 'A'
bootMsgEnd db '...',13,10,0

times 510-($-$$) db 0
dw 0xaa55

