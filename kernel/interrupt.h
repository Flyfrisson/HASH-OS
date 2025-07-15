#ifndef HASHOS_INTERRUPT_H
#define HASHOS_INTERRUPT_H

void init_interrupts();
void remap_pic();
void load_idt();
void timer_interrupt_handler();

#endif
