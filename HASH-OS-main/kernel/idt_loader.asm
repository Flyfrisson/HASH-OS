; IDT loader (32-bit)
[bits 32]
[global load_idt_asm]

load_idt_asm:
    mov eax, [esp + 4]  ; IDT pointer address
    lidt [eax]
    ret
