; kernel/entry.asm

BITS 32

section .multiboot
align 4
    dd 0x1BADB002          ; Multiboot magic number
    dd 0x00                ; Flags
    dd -(0x1BADB002 + 0x00) ; Checksum

section .text
global start
extern kernel_main

_start:
    mov [boot_fb_addr], eax    ; Save framebuffer address from bootloader
    push eax                   ; Pass framebuffer address to C kernel
    call kernel_main           ; Call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 4
boot_fb_addr: resd 1           ; Reserve 4 bytes for framebuffer address
