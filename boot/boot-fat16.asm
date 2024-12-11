[org 0x7c00]
[bits 16]

fat equ 0x7e00
mbr equ 0x8000
root_dir equ 0x9000
kernel_offs equ 0xa00
boot_drive equ 0x900
partition_entry_start equ 0x1be
partition_active equ 0x80
partition_inactive equ 0x00
dap_sector_start_2 equ 0x920
lba_start equ 0x910
root_sector equ 0x912
data_start equ 0x914
data_cluster equ 0x916


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
drive_number db  0x00
reserved_1 db  0x00
boot_signature db  0x00
volume_id dd  0x00000000
volume_label db 'NEW DISK   '
fat_type db 'FAT16   '

start:
xor ax, ax
mov ss, ax
mov sp, 0x7c00
sti
push ax
pop es
push ax
pop ds
cld
;mov dl, 0x80
mov [boot_drive], dl

codestart:
mov cx, 1
mov bx, mbr
xor dx, dx
mov ax, 0
mov si, 0
call diskread
add bx, partition_entry_start
mov cx, 0
mov ah, 0
findpartition:
mov al, [bx]
cmp al, partition_active
jnz nextpartition
loadpartition:
add bx, 8
mov ax, word [bx]
mov word [lba_start], ax
mov bx, lba_start
mov dx, word [bx]
mov ax, dx
mov bx, reserved_sectors_count
mov dx, word [bx]
add ax, dx
mov dx, ax
push dx
mov ax, word [number_fats]
mov bx, ax
mov ax, word [fat_size_16]
imul bx
pop dx
add dx, ax
mov ax, dx
mov word [root_sector], ax

; startsector = lba_start + reserved_sectors_count
; fatsectors = fat_size_16 * number_fats
; rootsector = startsector + fatsectors

mov ax, 0
mov bx, root_dir
mov cx, 8
mov si, 0
call diskread
mov ax, word [root_sector]
mov bx, 0
findfile:
mov si, root_dir
add si, bx
mov di, filename
mov cx, 11
repe cmpsb
jz readfile
add bx, 32
cmp bx, 4064
jle findfile
call errormsg
ret

readfile:
add bx, root_dir
add bx, 11
mov dl, byte [bx]
cmp dl, 0x08
jz errormsg
cmp dl, 0x0F
jz errormsg
cmp dl, 0x10
jz errormsg
add bx, 15
mov dx, word [bx]
mov word [data_cluster], dx
mov ax, 32
mov dx, word [root_entries_count]
mul dx
mov dx, word [bytes_per_sector]
add ax, dx
dec ax 
mov bx, word [bytes_per_sector]
mov dx, 0
div bx
xor dx, dx
add ax, word [root_sector]
mov word [data_start], ax
mov ax, word [data_cluster]
sub ax, 2
xor dx, dx
mov dl, byte [sector_per_cluster]
mul dx
add ax, word [data_start]
mov cx, 127
mov bx, 0
xchg ax, dx
mov si, kernel_offs
call diskread
;mov bx, kernel_offs
;mov dx, word [bx]
call loadmsg
call loadkernel
jmp $

loadkernel:
mov ax, 0
push ax
call kernel_offs:0
;hlt
jmp $

nextpartition:
add bx, 16
inc cx
cmp cx, 4
jz errormsg
ret

diskread:
	mov word [dap_sector_start_2], ax
	pusha
	mov ax, cx
	mov word [dap_sector_count], ax
	mov cx, 0
	mov ax, bx
	mov word [dap_sector_buffer], ax
	mov ax, si
	mov word [dap_sector_buffer+2], ax
	mov ax, dx
	mov word [dap_lba_start], ax
	mov ax, word [dap_sector_start_2]
	mov word [dap_lba_start+2], ax
	popa
	mov si, dap
	mov ah, 0x42
	mov dl, [boot_drive]
	int 0x13
	jc errormsg
	ret

errormsg:
mov bx, msgError
call print
;hlt
jmp $

loadmsg:
mov bx, MsgLoad
call print
ret

print:
pusha
print_next:
mov al, [bx]
cmp al, 0
je print_end
;call putchar
mov ah, 0x0e
int 0x10
add bx, 1
jmp print_next
print_end:
popa
ret

;print_n:
;mov al, 0x0d
;call putchar
;mov al, 0x0a
;call putchar
;ret

;putchar:
;mov ah, 0x0e
;int 0x10
;ret



dap:
					db	0x10
					db	0
dap_sector_count:	dw	1
dap_sector_buffer:	dw	0x7C00
					dw	0
dap_lba_start:		dd	1
					dd	0
					


msgError db 'LOADER not found.',13,10,0
MsgLoad db  'Starting FastSystem...',13,10,0
filename db 'LOADER     ',0

times 510-($-$$) db 0
dw 0xaa55

