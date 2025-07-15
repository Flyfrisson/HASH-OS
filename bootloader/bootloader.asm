[org 0x7C00]
[bits 16]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, boot_msg
    call print_string

    ; Load second stage (Multiboot kernel at 0x100000)
    mov eax, 0x100000        ; Load address for kernel
    mov es, ax
    xor bx, bx               ; Offset = 0x0000
    mov ah, 0x02             ; BIOS read sector
    mov al, 20               ; Number of sectors to read
    mov ch, 0                ; Cylinder
    mov cl, 2                ; Sector (start from 2)
    mov dh, 0                ; Head
    mov dl, 0x80             ; First hard disk
    int 0x13
    jc disk_error

    ; Jump to loaded kernel (Multiboot header + GRUB will handle properly)
    jmp 0x0000:0x0000

disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.next:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .next
.done:
    ret

boot_msg db "Booting HASH OS...", 0
error_msg db "Disk read error!", 0

times 510 - ($ - $$) db 0
dw 0xAA55