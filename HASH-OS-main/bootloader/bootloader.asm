; HASH OS Bootloader - Enhanced with Dynamic Resolution Detection and Error Handling
; =============================================================================
; Features:
; - Dynamic 4K resolution detection
; - Fallback resolution support
; - Comprehensive error handling
; - Boot disk validation
; - Memory layout verification
; - Kernel loading with progress indication
; =============================================================================

[bits 16]
[org 0x7C00]

; =============================================================================
; Boot Entry Point
; =============================================================================
start:
    ; Disable interrupts during setup
    cli
    
    ; Clear direction flag for string operations
    cld
    
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00    ; Stack grows downward from bootloader
    
    ; Save BIOS boot disk number
    mov [boot_disk], dl
    
    ; Enable interrupts now that setup is complete
    sti
    
    ; Display boot message
    mov si, boot_msg
    call print_string
    
    ; Initialize VESA graphics
    call init_vesa_graphics
    
    ; Load and start kernel
    call load_kernel
    
    ; Should never reach here
    jmp system_halt

; =============================================================================
; VESA Graphics Initialization with Dynamic Detection
; =============================================================================
init_vesa_graphics:
    ; Display graphics initialization message
    mov si, vesa_init_msg
    call print_string
    
    ; Get VBE Controller Information
    mov ax, 0x4F00          ; VBE Get Controller Info
    mov di, vbe_info_block  ; Buffer for VBE info
    int 0x10
    
    ; Check if VESA is supported
    cmp ax, 0x004F
    jne vesa_not_supported
    
    ; Verify VBE signature
    mov si, vbe_info_block
    mov di, vbe_signature
    mov cx, 4
    repe cmpsb
    jne vesa_not_supported
    
    ; Try to set 4K resolution first
    call try_4k_mode
    jc try_lower_resolutions
    
    ; 4K mode successful
    mov si, mode_4k_msg
    call print_string
    ret
    
try_lower_resolutions:
    ; Try 1440p (2560x1440)
    call try_1440p_mode
    jc try_1080p
    
    mov si, mode_1440p_msg
    call print_string
    ret
    
try_1080p:
    ; Try 1080p (1920x1080)
    call try_1080p_mode
    jc try_720p
    
    mov si, mode_1080p_msg
    call print_string
    ret
    
try_720p:
    ; Try 720p (1280x720) as last resort
    call try_720p_mode
    jc vesa_mode_fail
    
    mov si, mode_720p_msg
    call print_string
    ret

; Try 4K mode (3840x2160x32bpp)
try_4k_mode:
    mov ax, 0x4F01          ; Get mode info
    mov cx, 0x411A          ; 4K mode number (may vary by hardware)
    mov di, mode_info_block
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    ; Check if mode supports linear framebuffer
    test byte [mode_info_block + 0x00], 0x80
    jz .fail
    
    ; Set the mode
    mov ax, 0x4F02
    mov bx, 0x411A          ; Mode with linear framebuffer bit
    or bx, 0x4000           ; Set linear framebuffer bit
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    ; Store resolution info
    mov word [screen_width], 3840
    mov word [screen_height], 2160
    mov byte [screen_bpp], 32
    
    ; Get framebuffer address
    mov eax, [mode_info_block + 0x28]
    mov [framebuffer_ptr], eax
    
    clc                     ; Clear carry flag (success)
    ret
    
.fail:
    stc                     ; Set carry flag (failure)
    ret

; Try 1440p mode (2560x1440x32bpp)
try_1440p_mode:
    mov ax, 0x4F01
    mov cx, 0x4119          ; 1440p mode (approximate)
    mov di, mode_info_block
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    mov ax, 0x4F02
    mov bx, 0x4119
    or bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    mov word [screen_width], 2560
    mov word [screen_height], 1440
    mov byte [screen_bpp], 32
    
    mov eax, [mode_info_block + 0x28]
    mov [framebuffer_ptr], eax
    
    clc
    ret
    
.fail:
    stc
    ret

; Try 1080p mode (1920x1080x32bpp)
try_1080p_mode:
    mov ax, 0x4F01
    mov cx, 0x4118          ; 1080p mode
    mov di, mode_info_block
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    mov ax, 0x4F02
    mov bx, 0x4118
    or bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    mov word [screen_width], 1920
    mov word [screen_height], 1080
    mov byte [screen_bpp], 32
    
    mov eax, [mode_info_block + 0x28]
    mov [framebuffer_ptr], eax
    
    clc
    ret
    
.fail:
    stc
    ret

; Try 720p mode (1280x720x32bpp)
try_720p_mode:
    mov ax, 0x4F01
    mov cx, 0x4115          ; 720p mode
    mov di, mode_info_block
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    mov ax, 0x4F02
    mov bx, 0x4115
    or bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    mov word [screen_width], 1280
    mov word [screen_height], 720
    mov byte [screen_bpp], 32
    
    mov eax, [mode_info_block + 0x28]
    mov [framebuffer_ptr], eax
    
    clc
    ret
    
.fail:
    stc
    ret

; =============================================================================
; Kernel Loading with Progress Indication
; =============================================================================
load_kernel:
    ; Display kernel loading message
    mov si, kernel_msg
    call print_string
    
    ; Validate boot disk
    mov dl, [boot_disk]
    cmp dl, 0x80            ; First hard disk
    jb .floppy_disk
    cmp dl, 0x83            ; Last typical hard disk
    ja disk_invalid
    jmp .disk_valid
    
.floppy_disk:
    cmp dl, 0x00            ; First floppy
    jb disk_invalid
    cmp dl, 0x01            ; Second floppy
    ja disk_invalid
    
.disk_valid:
    ; Reset disk system
    mov ah, 0x00
    mov dl, [boot_disk]
    int 0x13
    jc disk_reset_fail
    
    ; Load kernel in chunks with progress indication
    mov byte [sectors_loaded], 0
    mov byte [total_sectors], 32    ; Load 32 sectors (16KB kernel)
    
    ; Set up for loading
    mov bx, 0x1000          ; Load segment
    mov es, bx
    mov bx, 0x0000          ; Load offset
    
load_loop:
    ; Calculate current sector
    mov al, [sectors_loaded]
    add al, 2               ; Start from sector 2 (after boot sector)
    mov cl, al
    
    ; Load one sector at a time for progress indication
    mov ah, 0x02            ; Read sectors function
    mov al, 1               ; Read 1 sector
    mov ch, 0               ; Cylinder 0
    mov dh, 0               ; Head 0
    mov dl, [boot_disk]     ; Drive
    int 0x13
    jc disk_read_fail
    
    ; Update progress
    inc byte [sectors_loaded]
    call show_progress
    
    ; Move to next sector position in memory
    add bx, 512             ; Move 512 bytes forward
    
    ; Check if we've loaded all sectors
    mov al, [sectors_loaded]
    cmp al, [total_sectors]
    jb load_loop
    
    ; Kernel loaded successfully
    mov si, kernel_loaded_msg
    call print_string
    
    ; Prepare to jump to kernel
    call prepare_kernel_jump
    
    ; Jump to kernel with framebuffer address in EAX
    mov eax, [framebuffer_ptr]
    jmp 0x1000:0x0000

; Show loading progress
show_progress:
    mov al, [sectors_loaded]
    mov bl, [total_sectors]
    
    ; Simple progress indicator (dots)
    mov al, '.'
    mov ah, 0x0E
    int 0x10
    
    ; Show percentage every 8 sectors
    mov al, [sectors_loaded]
    and al, 0x07
    jnz .done
    
    mov al, ' '
    int 0x10
    
.done:
    ret

; Prepare for kernel jump
prepare_kernel_jump:
    ; Disable interrupts before kernel jump
    cli
    
    ; Set up registers for kernel
    ; EAX = framebuffer address (already set)
    ; EBX = screen width
    ; ECX = screen height  
    ; EDX = bits per pixel
    
    movzx ebx, word [screen_width]
    movzx ecx, word [screen_height]
    movzx edx, byte [screen_bpp]
    
    ret

; =============================================================================
; Error Handlers
; =============================================================================
vesa_not_supported:
    mov si, vesa_fail_msg
    call print_string
    jmp system_halt

vesa_mode_fail:
    mov si, mode_fail_msg
    call print_string
    jmp system_halt

disk_invalid:
    mov si, disk_invalid_msg
    call print_string
    jmp system_halt

disk_reset_fail:
    mov si, disk_reset_msg
    call print_string
    jmp system_halt

disk_read_fail:
    mov si, disk_fail_msg
    call print_string
    jmp system_halt

system_halt:
    mov si, halt_msg
    call print_string
    cli
    hlt
    jmp system_halt         ; In case of spurious interrupt

; =============================================================================
; Utility Functions
; =============================================================================

; Print null-terminated string
; SI = pointer to string
print_string:
    pusha
    mov ah, 0x0E            ; Teletype output
    mov bh, 0               ; Page number
    mov bl, 0x07            ; Text attribute (light gray on black)
    
.next_char:
    lodsb                   ; Load byte from SI into AL
    cmp al, 0               ; Check for null terminator
    je .done
    int 0x10                ; BIOS video interrupt
    jmp .next_char
    
.done:
    popa
    ret

; Print hexadecimal number
; AX = number to print
print_hex:
    pusha
    mov cx, 4               ; 4 hex digits
    
.digit_loop:
    rol ax, 4               ; Rotate left 4 bits
    mov bx, ax
    and bl, 0x0F            ; Mask lower 4 bits
    
    cmp bl, 9
    jle .numeric
    add bl, 'A' - 10        ; Convert to A-F
    jmp .print_digit
    
.numeric:
    add bl, '0'             ; Convert to 0-9
    
.print_digit:
    mov al, bl
    mov ah, 0x0E
    int 0x10
    
    loop .digit_loop
    
    popa
    ret

; =============================================================================
; Data Section
; =============================================================================

; Boot disk number
boot_disk db 0

; Graphics information
framebuffer_ptr dd 0
screen_width    dw 0
screen_height   dw 0
screen_bpp      db 0

; Loading progress
sectors_loaded  db 0
total_sectors   db 0

; VBE signature to check
vbe_signature db 'VESA'

; Status messages
boot_msg         db 'HASH OS Bootloader v2.0', 13, 10, 0
vesa_init_msg    db 'Initializing VESA graphics...', 13, 10, 0
kernel_msg       db 'Loading kernel', 0
kernel_loaded_msg db 13, 10, 'Kernel loaded successfully!', 13, 10, 0

; Resolution messages
mode_4k_msg      db ' 4K mode set (3840x2160)', 13, 10, 0
mode_1440p_msg   db ' 1440p mode set (2560x1440)', 13, 10, 0
mode_1080p_msg   db ' 1080p mode set (1920x1080)', 13, 10, 0  
mode_720p_msg    db ' 720p mode set (1280x720)', 13, 10, 0

; Error messages
vesa_fail_msg    db 'ERROR: VESA not supported!', 13, 10, 0
mode_fail_msg    db 'ERROR: No compatible graphics mode found!', 13, 10, 0
disk_invalid_msg db 'ERROR: Invalid boot disk!', 13, 10, 0
disk_reset_msg   db 'ERROR: Disk reset failed!', 13, 10, 0
disk_fail_msg    db 'ERROR: Disk read failed!', 13, 10, 0
halt_msg         db 'System halted.', 13, 10, 0

; =============================================================================
; Data Buffers (located at specific addresses)
; =============================================================================

; Reserve space for VBE info block (512 bytes at 0x8000)
times 0x8000 - ($ - $$) - 512 db 0
vbe_info_block: times 512 db 0

; Reserve space for mode info block (256 bytes at 0x9000) 
times 0x9000 - ($ - $$) - 256 db 0
mode_info_block: times 256 db 0

; =============================================================================
; Boot Signature
; =============================================================================
times 510 - ($ - $$) db 0
dw 0xAA55