#include "interrupt.h"

// Stub IDT entry structure
struct IDTEntry {
    unsigned short base_low;
    unsigned short selector;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

struct IDTPointer {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

extern void load_idt_asm(unsigned int);

void init_interrupts() {
    idt_ptr.limit = (sizeof(struct IDTEntry) * 256) - 1;
    idt_ptr.base = (unsigned int)&idt;

    for (int i = 0; i < 256; i++) {
        idt[i].base_low = 0;
        idt[i].selector = 0x08;
        idt[i].always0 = 0;
        idt[i].flags = 0x8E;
        idt[i].base_high = 0;
    }

    // Timer IRQ 0 will be set here later

    load_idt();
}

void load_idt() {
    load_idt_asm((unsigned int)&idt_ptr);
}

void timer_interrupt_handler() {
    // In future: Add task switching call here
}
