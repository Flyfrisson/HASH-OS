OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
    /* Load the kernel at 1 MB */
    . = 1M;

    /* Kernel code section */
    .text :
    {
        *(.multiboot)
        *(.text)
    }

    /* Kernel read-only data section */
    .rodata :
    {
        *(.rodata)
    }

    /* Kernel data section */
    .data :
    {
        *(.data)
    }

    /* Kernel BSS section (uninitialized data) */
    .bss :
    {
        *(.bss)
    }
}
