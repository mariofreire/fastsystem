[bits 32]

section .text
global getdrivenumber
global diskread
global probememory
global int86_start
global int86_end
global int86_regs
global int86_int_rsrv
global int86_int_instr
global int86_int_no
global int386_regs
global int386_start
global int386_end
global int386_int_rsrv
global int386_int_instr
global int386_int_no
global halt
global switchtokernelmode
global switchtousermode
global shell_code_exec
global shell_code_exit
global kernelmode_start
global pic_set
global pic_restore
global pic_set_2
global pic_restore_2
global load_page_directory
global enable_paging
global enable_pse
global enable_sse
global current_code_segment
global current_data_segment
global syscall_stack_count
global syscall_stack_start
global syscall_last_base
global syscall_last_source
global syscall_last_dest
global syscall_last_data
global syscall_last_base1
global syscall_last_count
global syscall_last_acc
extern printhex
extern isr_handler
extern irq_handler
extern interrupt_handler
extern kernelmode_preinit
extern kernelmode_init
extern usermode_init
extern reload_devices
extern remap_irq


%define INT86_BASE_ADDRESS 0x7C00
%define get_base_address(x)  (((x) - int86_start) + INT86_BASE_ADDRESS)
%define get_base_address_int386(x)  (((x) - int386_start) + INT86_BASE_ADDRESS)


%macro isr_noerr 1
global isr%1
isr%1:
	cli
	push byte 0
	push %1
	jmp isr_stub
%endmacro

%macro isr_err 1
global isr%1
isr%1:
	cli
	push %1
	jmp isr_stub
%endmacro

%macro irq 2
global irq%1
irq%1:
	cli
	push byte 0
	push byte %2
	jmp irq_stub
%endmacro


halt:
	hlt
	jmp $


shell_code_exit:
	mov al, byte [current_data_segment]
	mov ds, ax
	mov es, ax
	mov fs, ax
    mov gs, ax
	mov ecx, dword [syscall_stack_count]
	mov edx, syscall_stack_start
	add edx, ecx
	add edx, 4
	mov eax, dword [edx]
	mov esp, eax
	mov ecx, [eax]
	mov eax, ecx
	xor ecx, ecx
	mov ebp, dword [syscall_last_base]
	mov esi, dword [syscall_last_source]
	mov edi, dword [syscall_last_dest]
	mov edx, dword [syscall_last_data]
	mov ebx, dword [syscall_last_base1]
	mov ecx, dword [syscall_last_count]
	xor ebx, ebx
	xor ecx, ecx
    mov bl, byte [current_code_segment]
	mov cl, byte [current_data_segment]
    push ecx
    push esp
    pushfd
    push ebx
	push eax
	mov ebx, dword [syscall_last_base1]
	mov ecx, dword [syscall_last_count]	
	mov eax, dword [syscall_last_acc]
	; call reload_devices
	iretd
	hlt
	jmp $
	
shell_code_exec:
	mov dword [syscall_last_base], ebp
	mov dword [syscall_last_source], esi
	mov dword [syscall_last_dest], edi
	mov dword [syscall_last_data], edx
	mov dword [syscall_last_base1], ebx
	mov dword [syscall_last_count], ecx
	mov dword [syscall_last_acc], eax
	mov eax, esp
	mov ecx, dword [syscall_stack_count]
	dec ecx
	mov dword [syscall_stack_count], ecx
	mov edx, syscall_stack_start
	add edx, ecx
	mov ecx, edx
	xor edx, edx
	mov dword [ecx + 4], eax
	push ebp
	mov ebp, esp
	mov eax, dword [ebp + 8]
	mov edi, dword [ebp + 12]
	mov esi, dword [ebp + 16]
	pop ebp
	call eax
	jmp $

kernelmode_start:
	add esp, 4
	mov eax, 0
	mov ebx, 0
	mov ecx, 0
	mov edx, 0
	call kernelmode_init
	mov esi, 0
	mov eax, 0
	mov ebx, 1
	mov edx, 0
	mov ecx, 0
	ret

switchtokernelmode:	
	call kernelmode_preinit
	mov ax, KERNEL_MODE_CODE_SEGMENT	
	mov byte [current_code_segment], al
	mov ax, KERNEL_MODE_DATA_SEGMENT
	mov byte [current_data_segment], al
	
	mov dword [syscall_last_base], ebp
	mov dword [syscall_last_source], esi
	mov dword [syscall_last_dest], edi
	mov dword [syscall_last_data], edx
	mov dword [syscall_last_base1], ebx
	mov dword [syscall_last_count], ecx
	mov dword [syscall_last_acc], eax
	mov eax, esp
	mov ecx, dword [syscall_stack_count]
	dec ecx
	mov dword [syscall_stack_count], ecx
	mov edx, syscall_stack_start
	add edx, ecx
	mov ecx, edx
	xor edx, edx
	mov dword [ecx + 4], eax
	call kernelmode_init
	mov eax, 1
	mov ebx, 0
	int 0x80
	jmp KERNEL_MODE_CODE_SEGMENT:kernelmode_code
	hlt
	jmp $
	
kernelmode_code:
	mov ax, KERNEL_MODE_CODE_SEGMENT	
	mov byte [current_code_segment], al
	mov ax, KERNEL_MODE_DATA_SEGMENT
	mov byte [current_data_segment], al
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push KERNEL_MODE_DATA_SEGMENT
    push esp
    pushfd
    push KERNEL_MODE_CODE_SEGMENT
    lea eax, [kernelmode_start]
	push eax
	sti
    iretd
	hlt
	
switchtousermode:
	mov ax, USER_MODE_CODE_SEGMENT	
	mov byte [current_code_segment], al
	mov ax, USER_MODE_DATA_SEGMENT
	mov byte [current_data_segment], al
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push USER_MODE_DATA_SEGMENT
    push esp
    pushfd
    push USER_MODE_CODE_SEGMENT
    lea eax, [usermode_start]
	push eax
	sti
    iretd
	hlt
	jmp $

usermode_start:
	add esp, 4
	mov eax, 0
	mov ebx, 0
	mov ecx, 0
	mov edx, 0
	call usermode_init
	mov esi, 0
	mov eax, 0
	mov ebx, 1
	mov edx, 0
	mov ecx, 0
	ret


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

isr_stub:
	pusha
	mov ax, ds
	push eax
	xor eax, eax
	mov al, byte [current_data_segment]
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	push esp
	call isr_handler
	pop esp
	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	popa
	add esp, 8
	sti
	iret

irq_stub:
	pusha
	mov ax, ds
	push eax
	xor eax, eax
	mov al, byte [current_data_segment]
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	push esp
	call irq_handler
	pop esp
	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	popa
	add esp, 8
	sti
	iret
	

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31


irq 0, 32
irq 1, 33
irq 2, 34
irq 3, 35
irq 4, 36
irq 5, 37
irq 6, 38
irq 7, 39
irq 8, 40
irq 9, 41
irq 10, 42
irq 11, 43
irq 12, 44
irq 13, 45
irq 14, 46
irq 15, 47



isr_noerr 48
isr_noerr 49
isr_noerr 50
isr_noerr 51
isr_noerr 52
isr_noerr 53
isr_noerr 54
isr_noerr 55
isr_noerr 56
isr_noerr 57
isr_noerr 58
isr_noerr 59
isr_noerr 60
isr_noerr 61
isr_noerr 62
isr_noerr 63
isr_noerr 64
isr_noerr 65
isr_noerr 66
isr_noerr 67
isr_noerr 68
isr_noerr 69
isr_noerr 70
isr_noerr 71
isr_noerr 72
isr_noerr 73
isr_noerr 74
isr_noerr 75
isr_noerr 76
isr_noerr 77
isr_noerr 78
isr_noerr 79
isr_noerr 80
isr_noerr 81
isr_noerr 82
isr_noerr 83
isr_noerr 84
isr_noerr 85
isr_noerr 86
isr_noerr 87
isr_noerr 88
isr_noerr 89
isr_noerr 90
isr_noerr 91
isr_noerr 92
isr_noerr 93
isr_noerr 94
isr_noerr 95
isr_noerr 96
isr_noerr 97
isr_noerr 98
isr_noerr 99
isr_noerr 100
isr_noerr 101
isr_noerr 102
isr_noerr 103
isr_noerr 104
isr_noerr 105
isr_noerr 106
isr_noerr 107
isr_noerr 108
isr_noerr 109
isr_noerr 110
isr_noerr 111
isr_noerr 112
isr_noerr 113
isr_noerr 114
isr_noerr 115
isr_noerr 116
isr_noerr 117
isr_noerr 118
isr_noerr 119
isr_noerr 120
isr_noerr 121
isr_noerr 122
isr_noerr 123
isr_noerr 124
isr_noerr 125
isr_noerr 126
isr_noerr 127
isr_noerr 128
isr_noerr 129
isr_noerr 130
isr_noerr 131
isr_noerr 132
isr_noerr 133
isr_noerr 134
isr_noerr 135
isr_noerr 136
isr_noerr 137
isr_noerr 138
isr_noerr 139
isr_noerr 140
isr_noerr 141
isr_noerr 142
isr_noerr 143
isr_noerr 144
isr_noerr 145
isr_noerr 146
isr_noerr 147
isr_noerr 148
isr_noerr 149
isr_noerr 150
isr_noerr 151
isr_noerr 152
isr_noerr 153
isr_noerr 154
isr_noerr 155
isr_noerr 156
isr_noerr 157
isr_noerr 158
isr_noerr 159
isr_noerr 160
isr_noerr 161
isr_noerr 162
isr_noerr 163
isr_noerr 164
isr_noerr 165
isr_noerr 166
isr_noerr 167
isr_noerr 168
isr_noerr 169
isr_noerr 170
isr_noerr 171
isr_noerr 172
isr_noerr 173
isr_noerr 174
isr_noerr 175
isr_noerr 176
isr_noerr 177
isr_noerr 178
isr_noerr 179
isr_noerr 180
isr_noerr 181
isr_noerr 182
isr_noerr 183
isr_noerr 184
isr_noerr 185
isr_noerr 186
isr_noerr 187
isr_noerr 188
isr_noerr 189
isr_noerr 190
isr_noerr 191
isr_noerr 192
isr_noerr 193
isr_noerr 194
isr_noerr 195
isr_noerr 196
isr_noerr 197
isr_noerr 198
isr_noerr 199
isr_noerr 200
isr_noerr 201
isr_noerr 202
isr_noerr 203
isr_noerr 204
isr_noerr 205
isr_noerr 206
isr_noerr 207
isr_noerr 208
isr_noerr 209
isr_noerr 210
isr_noerr 211
isr_noerr 212
isr_noerr 213
isr_noerr 214
isr_noerr 215
isr_noerr 216
isr_noerr 217
isr_noerr 218
isr_noerr 219
isr_noerr 220
isr_noerr 221
isr_noerr 222
isr_noerr 223
isr_noerr 224
isr_noerr 225
isr_noerr 226
isr_noerr 227
isr_noerr 228
isr_noerr 229
isr_noerr 230
isr_noerr 231
isr_noerr 232
isr_noerr 233
isr_noerr 234
isr_noerr 235
isr_noerr 236
isr_noerr 237
isr_noerr 238
isr_noerr 239
isr_noerr 240
isr_noerr 241
isr_noerr 242
isr_noerr 243
isr_noerr 244
isr_noerr 245
isr_noerr 246
isr_noerr 247
isr_noerr 248
isr_noerr 249
isr_noerr 250
isr_noerr 251
isr_noerr 252
isr_noerr 253
isr_noerr 254
isr_noerr 255


use16
int86_regs:    
	int86_r_di dw 0x0000
	int86_r_si dw 0x0000
	int86_r_bp dw 0x0000
	int86_r_sp dw 0x0000
	int86_r_bx dw 0x0000
	int86_r_dx dw 0x0000
	int86_r_cx dw 0x0000
	int86_r_ax dw 0x0000
	int86_r_gs dw 0x0000
	int86_r_fs dw 0x0000
	int86_r_es dw 0x0000
	int86_r_ds dw 0x0000
	int86_r_fl dw 0x0000

use32
int86_start:
	cli
	pusha
	mov eax, esp
	mov dword [get_base_address(int86_esp)], eax
	xor eax, eax
	sidt [get_base_address(int86_idt32)]
	mov eax, cr0
	and eax, ~0x80000000
	mov cr0, eax
	xor ecx, ecx
	mov ebx, cr3
	mov dword [get_base_address(int86_cr3)], ebx	
	mov cr3, ecx
	jmp word CODE16_SEGMENT:get_base_address(int86_pmode16)
	
int86_pmode16:
use16
	mov ax, DATA16_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov eax, cr0
	and eax, ~0x01
	mov cr0, eax
	jmp word 0x00:get_base_address(int86_rmode16)
int86_rmode16:
use16
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax	
	lidt [get_base_address(int86_idt16)]	
	pusha
	lea ax, [get_base_address(int86_regs)]
	mov sp, ax  
	call pic_set
	popa	
	sti
int86_int_rsrv: nop
	nop
	nop
int86_int_instr: db 0xCD
int86_int_no: db 0x00
	cli
	xor sp, sp
	mov ss, sp
	lea sp, [get_base_address(int86_regs)+16]
    pusha  
	call pic_restore
	lea sp, [get_base_address(int86_regs)]
	popa
	mov sp, INT86_BASE_ADDRESS         
	pushf
	mov eax, cr0
	inc eax
	mov cr0, eax
	jmp dword CODE_SEGMENT:get_base_address(int86_pmode32)

int86_pmode32:
use32
	mov ax, DATA_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov eax, dword [get_base_address(int86_esp)]
	mov esp, eax
	lidt [get_base_address(int86_idt32)]    
	mov ebx, dword [get_base_address(int86_cr3)]
	mov cr3, ebx
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	popa                                   
	sti                                    
	ret           

pic_set:
	mov  bx, 0x0870
	call pic_reset
	ret
		
pic_restore:
	mov  bx, 0x2028
	call pic_reset
	ret
	
pic_reset:
	push ax
	mov  al, 0x11 
	out  0x20, al  
	out  0xA0, al  
	mov  al, bh    
	out  0x21, al  
	mov  al, bl     
	out  0xA1, al         
	mov  al, 0x04  
	out  0x21, al     
	shr  al, 1   
	out  0xA1, al     
	shr  al, 1                
	out  0x21, al                  
	out  0xA1, al                  
	pop  ax                           
	ret

int86_esp:
	dd 0x00000000

int86_cr3:
	dd 0x00000000

int86_idt16:                                 
	dw 0x03FF
	dd 0x00000000
	
int86_idt32:                                 
	dw 0x0000
	dd 0x00000000

use32

int86_end:




use16
int386_regs:    
	int386_r_edi    dd 0
	int386_r_esi    dd 0
	int386_r_ebp    dd 0
	int386_r_esp    dd 0
	int386_r_ebx    dd 0
	int386_r_edx    dd 0
	int386_r_ecx    dd 0
	int386_r_eax    dd 0
	int386_r_ds     dw 0 
	int386_r_es     dw 0
	int386_r_fs     dw 0
	int386_r_gs     dw 0
	int386_r_ss     dw 0
	int386_r_eflags dd 0
	int386_r_pad    dw 0

use32
int386_start:
	cli
	pusha
	mov eax, esp
	mov dword [get_base_address_int386(int386_esp)], eax
	xor eax, eax
	sidt [get_base_address_int386(int386_idt32)]
	mov eax, cr0
	and eax, ~0x80000000
	mov cr0, eax
	xor ecx, ecx
	mov ebx, cr3
	mov dword [get_base_address_int386(int386_cr3)], ebx	
	mov cr3, ecx
	jmp word CODE16_SEGMENT:get_base_address_int386(int386_pmode16)
	
int386_pmode16:
use16
	mov ax, DATA16_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov eax, cr0
	and eax, ~0x01
	mov cr0, eax
	jmp word 0x00:get_base_address_int386(int386_rmode16)
int386_rmode16:
use16
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax	
	lidt [get_base_address_int386(int386_idt16)]	
	pushad
	lea ax, [get_base_address_int386(int386_regs)]
	mov sp, ax  
	call pic_set_2
	popad	
	sti
int386_int_rsrv: nop
	nop
	nop
int386_int_instr: db 0xCD
int386_int_no: db 0x00
	cli
	xor sp, sp
	mov ss, sp
	lea sp, [get_base_address_int386(int386_regs)+32]
    pushad  
	call pic_restore_2
	lea sp, [get_base_address_int386(int386_regs)]
	popad
	mov sp, INT86_BASE_ADDRESS         
	pushf
	mov eax, cr0
	inc eax
	mov cr0, eax
	jmp dword CODE_SEGMENT:get_base_address_int386(int386_pmode32)

int386_pmode32:
use32
	mov ax, DATA_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov eax, dword [get_base_address_int386(int386_esp)]
	mov esp, eax
	lidt [get_base_address_int386(int386_idt32)]    
	mov ebx, dword [get_base_address_int386(int386_cr3)]
	mov cr3, ebx
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	popa                                   
	sti                                    
	ret           

pic_set_2:
	mov  bx, 0x0870
	call pic_reset_2
	ret
		
pic_restore_2:
	mov  bx, 0x2028
	call pic_reset_2
	ret
	
pic_reset_2:
	push ax
	mov  al, 0x11 
	out  0x20, al  
	out  0xA0, al  
	mov  al, bh    
	out  0x21, al  
	mov  al, bl     
	out  0xA1, al         
	mov  al, 0x04  
	out  0x21, al     
	shr  al, 1   
	out  0xA1, al     
	shr  al, 1                
	out  0x21, al                  
	out  0xA1, al                  
	pop  ax                           
	ret

int386_esp:
	dd 0x00000000

int386_cr3:
	dd 0x00000000

int386_idt16:                                 
	dw 0x03FF
	dd 0x00000000
	
int386_idt32:                                 
	dw 0x0000
	dd 0x00000000

use32

int386_end:
























section .text

load_page_directory:
push ebp
mov ebp, esp
mov eax, dword [esp+8]
mov cr3, eax
mov esp, ebp
pop ebp
ret

enable_paging:
push ebp
mov ebp, esp
mov eax, cr0
or eax, 0x80000000
mov cr0, eax
mov esp, ebp
pop ebp
ret

enable_pse:
push ebp
mov ebp, esp
mov eax, cr4
or eax, 0x00000010
mov cr4, eax
mov esp, ebp
pop ebp
ret

enable_sse:
push ebp
mov ebp, esp
mov eax, cr0
and ax, 0xFFFB
or ax, 2
mov cr0, eax
mov eax, cr4
or ax, 1536
mov eax, cr4
mov esp, ebp
pop ebp
ret

print32:
mov ebx, 0xb8000
print32_loop:
lodsb
or al, al
jz print32_done
or eax, 0x0700
mov word [ebx], ax
add ebx, 2
jmp print32_loop
print32_done:
ret


; section .data



syscall_stack_start:
times 16384 dd 0
syscall_stack_end:
syscall_stack_size equ syscall_stack_end-syscall_stack_start
syscall_stack_count dd 16383
syscall_last_base dd 0
syscall_last_source dd 0
syscall_last_dest dd 0
syscall_last_data dd 0
syscall_last_base1 dd 0
syscall_last_count dd 0
syscall_last_acc dd 0


CODE16_SEGMENT equ 0x30
DATA16_SEGMENT equ 0x38
CODE_SEGMENT equ 0x08
DATA_SEGMENT equ 0x10
TSS_RING equ 0x03
TSS_CODE_SEGMENT equ (CODE_SEGMENT|TSS_RING)
TSS_DATA_SEGMENT equ (DATA_SEGMENT|TSS_RING)
KERNEL_MODE_CODE_SEGMENT equ CODE_SEGMENT
KERNEL_MODE_DATA_SEGMENT equ DATA_SEGMENT
USER_MODE_CODE_SEGMENT equ 0x1b ; (KERNEL_MODE_DATA_SEGMENT|TSS_CODE_SEGMENT)
USER_MODE_DATA_SEGMENT equ 0x23 ; (KERNEL_MODE_DATA_SEGMENT|TSS_DATA_SEGMENT)

current_code_segment db KERNEL_MODE_CODE_SEGMENT
current_data_segment db KERNEL_MODE_DATA_SEGMENT

msg db 'Hello',0

section .bss
	resb 16384

section .note.GNU-stack
