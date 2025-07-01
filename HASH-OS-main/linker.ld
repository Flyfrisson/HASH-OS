/* HASH OS Kernel Linker Script - Enhanced Version
 * =================================================
 *
 */

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(start)

/* Define useful constants */
KERNEL_BASE = 0x100000;     /* 1MB - Standard kernel load address */
PAGE_SIZE = 0x1000;         /* 4KB page size */
STACK_SIZE = 0x4000;        /* 16KB stack size */

SECTIONS
{
    /* Set the base address to 1MB (multiboot standard) */
    . = KERNEL_BASE;
    
    /* Store the kernel start address */
    kernel_start = .;
    
    /* =================================================================
     * MULTIBOOT HEADER SECTION
     * Must be in the first 8KB of the kernel for GRUB to find it
     * ================================================================= */
    .multiboot ALIGN(4) :
    {
        *(.multiboot)
        *(.multiboot.*)
    }
    
    /* =================================================================
     * TEXT SECTION - Executable Code
     * ================================================================= */
    .text ALIGN(PAGE_SIZE) :
    {
        text_start = .;
        
        /* Entry point and early boot code */
        *(.text.boot)
        *(.text.early)
        
        /* Main kernel code */
        *(.text)
        *(.text.*)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.t.*)
        
        text_end = .;
    }
    
    /* =================================================================
     * READ-ONLY DATA SECTION
     * ================================================================= */
    .rodata ALIGN(PAGE_SIZE) :
    {
        rodata_start = .;
        
        /* String literals and constants */
        *(.rodata)
        *(.rodata.*)
        *(.rodata.str1.1)
        *(.rodata.str1.4)
        *(.rodata.str1.8)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.r.*)
        
        /* Exception handling data (read-only) */
        *(.eh_frame_hdr)
        *(.eh_frame)
        
        /* Global constructor/destructor tables */
        . = ALIGN(4);
        ctors_start = .;
        *(.ctors)
        *(.init_array)
        ctors_end = .;
        
        . = ALIGN(4);
        dtors_start = .;
        *(.dtors)
        *(.fini_array)
        dtors_end = .;
        
        rodata_end = .;
    }
    
    /* =================================================================
     * INITIALIZED DATA SECTION
     * ================================================================= */
    .data ALIGN(PAGE_SIZE) :
    {
        data_start = .;
        
        /* Initialized global and static variables */
        *(.data)
        *(.data.*)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.d.*)
        
        /* Small data sections (for some architectures) */
        *(.sdata)
        *(.sdata.*)
        
        data_end = .;
    }
    
    /* =================================================================
     * UNINITIALIZED DATA SECTION (BSS)
     * ================================================================= */
    .bss ALIGN(PAGE_SIZE) :
    {
        bss_start = .;
        
        /* Uninitialized global and static variables */
        *(.bss)
        *(.bss.*)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.b.*)
        
        /* Common symbols */
        *(COMMON)
        
        /* Small BSS sections */
        *(.sbss)
        *(.sbss.*)
        
        /* Align to page boundary for memory management */
        . = ALIGN(PAGE_SIZE);
        bss_end = .;
    }
    
    /* =================================================================
     * KERNEL STACK
     * ================================================================= */
    .stack ALIGN(PAGE_SIZE) :
    {
        stack_bottom = .;
        . += STACK_SIZE;
        stack_top = .;
    }
    
    /* =================================================================
     * KERNEL HEAP START MARKER
     * The actual heap will be managed dynamically by the kernel
     * ================================================================= */
    .heap ALIGN(PAGE_SIZE) :
    {
        heap_start = .;
        /* Heap grows dynamically from here */
    }
    
    /* Store the kernel end address */
    kernel_end = .;
    
    /* Calculate kernel size */
    kernel_size = kernel_end - kernel_start;
    
    /* =================================================================
     * DEBUG INFORMATION SECTIONS
     * These sections are not loaded into memory but provide debug info
     * ================================================================= */
    
    /* DWARF debug information */
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_info     0 : { *(.debug_info) }
    .debug_line     0 : { *(.debug_line) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_ranges   0 : { *(.debug_ranges) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    
    /* Stabs debug information */
    .stab           0 : { *(.stab) }
    .stabstr        0 : { *(.stabstr) }
    
    /* Comment sections */
    .comment        0 : { *(.comment) }
    .note.GNU-stack 0 : { *(.note.GNU-stack) }
    
    /* =================================================================
     * DISCARDED SECTIONS
     * Remove unwanted sections that might be generated by the compiler
     * ================================================================= */
    /DISCARD/ :
    {
        /* Exception handling (not needed in kernel) */
        *(.eh_frame)
        *(.eh_frame_hdr)
        
        /* Thread-local storage (not supported) */
        *(.tdata)
        *(.tbss)
        
        /* Dynamic linking (not used in kernel) */
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.hash)
        *(.gnu.hash)
        *(.plt)
        *(.got)
        *(.got.plt)
        
        /* Interpreter section */
        *(.interp)
        
        /* Note sections we don't need */
        *(.note.*)
        
        /* GCC generated sections we don't need */
        *(.gcc_except_table)
        *(.gcc_except_table.*)
    }
    
    /* =================================================================
     * ASSERTIONS AND SANITY CHECKS
     * ================================================================= */
    
    /* Ensure kernel doesn't exceed reasonable size (16MB) */
    ASSERT(kernel_size < 0x1000000, "Kernel size exceeds 16MB")
    
    /* Ensure sections are properly aligned */
    ASSERT((text_start & (PAGE_SIZE - 1)) == 0, "Text section not page-aligned")
    ASSERT((rodata_start & (PAGE_SIZE - 1)) == 0, "Rodata section not page-aligned")
    ASSERT((data_start & (PAGE_SIZE - 1)) == 0, "Data section not page-aligned")
    ASSERT((bss_start & (PAGE_SIZE - 1)) == 0, "BSS section not page-aligned")
    
    /* Ensure stack is properly sized and aligned */
    ASSERT(stack_top - stack_bottom == STACK_SIZE, "Stack size mismatch")
    ASSERT((stack_bottom & (PAGE_SIZE - 1)) == 0, "Stack not page-aligned")
    
    /* Ensure kernel starts at the expected address */
    ASSERT(kernel_start == KERNEL_BASE, "Kernel not loaded at expected address")
}

/* =================================================================
 * PROVIDE EXTERNAL SYMBOLS
 * These symbols can be accessed from C code to get memory layout info
 * ================================================================= */

/* Memory layout symbols */
PROVIDE(kernel_physical_start = kernel_start);
PROVIDE(kernel_physical_end = kernel_end);
PROVIDE(kernel_virtual_start = kernel_start);
PROVIDE(kernel_virtual_end = kernel_end);

/* Section boundary symbols */
PROVIDE(__text_start = text_start);
PROVIDE(__text_end = text_end);
PROVIDE(__rodata_start = rodata_start);
PROVIDE(__rodata_end = rodata_end);
PROVIDE(__data_start = data_start);
PROVIDE(__data_end = data_end);
PROVIDE(__bss_start = bss_start);
PROVIDE(__bss_end = bss_end);

/* Stack symbols */
PROVIDE(__stack_bottom = stack_bottom);
PROVIDE(__stack_top = stack_top);

/* Heap symbols */
PROVIDE(__heap_start = heap_start);

/* Constructor/destructor symbols */
PROVIDE(__ctors_start = ctors_start);
PROVIDE(__ctors_end = ctors_end);
PROVIDE(__dtors_start = dtors_start);
PROVIDE(__dtors_end = dtors_end);

/* Kernel size symbol */
PROVIDE(__kernel_size = kernel_size);
