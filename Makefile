# HASH OS Kernel Makefile 
# =============================================================================

# Build tools
AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
MKDIR = mkdir -p
RM = rm -rf

# Directories
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
UI_DIR = ui
BUILD_DIR = build
INCLUDE_DIRS = $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR)

# Include paths for header files
INCLUDES = $(addprefix -I,$(INCLUDE_DIRS))

# Compiler flags with optimizations and warnings
CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -fno-stack-protector \
         -Wall -Wextra -Werror -O2 -g $(INCLUDES) \
         -fno-omit-frame-pointer -mno-red-zone

# Assembly flags
ASFLAGS = -f elf32 -g

# Linker flags
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary

# Source files organized by directory
KERNEL_SOURCES = kernel.c config_parser.c task.c interrupt.c fs.c fat.c app_manager.c
KERNEL_ASM_SOURCES = entry.asm idt_loader.asm
DRIVER_SOURCES = display4k.c driver.c audio_manager.c audio_profiles.c \
                 touch_input.c virtual_keyboard.c
UI_SOURCES = ui_manager.c file_explorer.c settings.c launcher.c
PLACEHOLDER_SOURCES = placeholders.c

# Object files with build directory paths
KERNEL_OBJS = $(addprefix $(BUILD_DIR)/,$(KERNEL_SOURCES:.c=.o))
KERNEL_ASM_OBJS = $(addprefix $(BUILD_DIR)/,$(KERNEL_ASM_SOURCES:.asm=.o))
DRIVER_OBJS = $(addprefix $(BUILD_DIR)/,$(DRIVER_SOURCES:.c=.o))
UI_OBJS = $(addprefix $(BUILD_DIR)/,$(UI_SOURCES:.c=.o))
PLACEHOLDER_OBJS = $(addprefix $(BUILD_DIR)/,$(PLACEHOLDER_SOURCES:.c=.o))

# All object files
ALL_OBJS = $(KERNEL_OBJS) $(KERNEL_ASM_OBJS) $(DRIVER_OBJS) $(UI_OBJS) $(PLACEHOLDER_OBJS)

# Target binary
TARGET = kernel.bin
TARGET_ELF = kernel.elf
TARGET_ISO = hash_os.iso

# Build configuration
BUILD_DATE = $(shell date '+%Y-%m-%d %H:%M:%S')
BUILD_VERSION = $(shell git describe --tags --always --dirty 2>/dev/null || echo "unknown")
BUILD_HASH = $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Parallel build support
MAKEFLAGS += --jobs=$(shell nproc 2>/dev/null || echo 4)

# Color definitions
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
WHITE = \033[0;37m
RESET = \033[0m

# =============================================================================
# Main Targets
# =============================================================================

.PHONY: all clean distclean info help debug release install iso test

# Default target
all: info $(TARGET)
	@printf "$(GREEN)✅ Build complete: $(TARGET) is ready.$(RESET)\n"
	@printf "$(CYAN)📊 Build size: %s$(RESET)\n" "$$(du -h $(TARGET) | cut -f1)"
	@printf "$(CYAN)🏗️  Build hash: $(BUILD_HASH)$(RESET)\n"

# Create kernel binary
$(TARGET): $(BUILD_DIR) $(ALL_OBJS) linker.ld
	@printf "$(YELLOW)🔗 Linking kernel...$(RESET)\n"
	$(LD) $(LDFLAGS) -o $(TARGET) $(ALL_OBJS)
	@printf "$(BLUE)📋 Kernel memory layout:$(RESET)\n"
	@$(LD) $(LDFLAGS) --print-map -o $(TARGET_ELF) $(ALL_OBJS) 2>/dev/null | head -20 || true
	@printf "$(GREEN)✅ Kernel linked successfully$(RESET)\n"

# Create ELF version for debugging
$(TARGET_ELF): $(BUILD_DIR) $(ALL_OBJS) linker.ld
	@printf "$(YELLOW)🔗 Creating ELF debug version...$(RESET)\n"
	$(LD) -m elf_i386 -T linker.ld -o $(TARGET_ELF) $(ALL_OBJS)

# Create bootable ISO
$(TARGET_ISO): $(TARGET)
	@printf "$(YELLOW)💿 Creating bootable ISO...$(RESET)\n"
	@if command -v grub-mkrescue >/dev/null 2>&1; then \
		mkdir -p iso/boot/grub; \
		cp $(TARGET) iso/boot/kernel.bin; \
		echo "menuentry \"HASH OS\" { multiboot /boot/kernel.bin }" > iso/boot/grub/grub.cfg; \
		grub-mkrescue -o $(TARGET_ISO) iso/; \
		printf "$(GREEN)✅ ISO created: $(TARGET_ISO)$(RESET)\n"; \
	else \
		printf "$(RED)❌ grub-mkrescue not found, skipping ISO creation$(RESET)\n"; \
	fi

iso: $(TARGET_ISO)

# Create build directory
$(BUILD_DIR):
	@printf "$(CYAN)📁 Creating build directory...$(RESET)\n"
	$(MKDIR) $(BUILD_DIR)

# =============================================================================
# Build Rules for C Files
# =============================================================================

# Kernel C files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@printf "$(CYAN)🔨 Compiling kernel: %s$(RESET)\n" "$<"
	$(CC) $(CFLAGS) -DBUILD_DATE="\"$(BUILD_DATE)\"" \
	      -DBUILD_VERSION="\"$(BUILD_VERSION)\"" \
	      -DBUILD_HASH="\"$(BUILD_HASH)\"" \
	      -c $< -o $@

# Driver C files
$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c
	@printf "$(CYAN)🔨 Compiling driver: %s$(RESET)\n" "$<"
	$(CC) $(CFLAGS) -c $< -o $@

# UI C files
$(BUILD_DIR)/%.o: $(UI_DIR)/%.c
	@printf "$(CYAN)🔨 Compiling UI: %s$(RESET)\n" "$<"
	$(CC) $(CFLAGS) -c $< -o $@

# Placeholder files (root directory)
$(BUILD_DIR)/%.o: %.c
	@printf "$(CYAN)🔨 Compiling placeholder: %s$(RESET)\n" "$<"
	$(CC) $(CFLAGS) -c $< -o $@

# =============================================================================
# Build Rules for Assembly Files
# =============================================================================

# Kernel assembly files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	@printf "$(PURPLE)🔧 Assembling: %s$(RESET)\n" "$<"
	$(AS) $(ASFLAGS) $< -o $@

# =============================================================================
# Special Targets
# =============================================================================

# Debug build with extra symbols and no optimization
debug: CFLAGS += -DDEBUG -O0 -ggdb3 -fsanitize-address-use-after-scope
debug: ASFLAGS += -DDEBUG
debug: clean $(TARGET_ELF)
	@printf "$(GREEN)🐛 Debug build completed$(RESET)\n"
	@printf "$(CYAN)📁 Debug symbols available in $(TARGET_ELF)$(RESET)\n"

# Release build with maximum optimization
release: CFLAGS += -DRELEASE -O3 -DNDEBUG -flto -fdata-sections -ffunction-sections
release: LDFLAGS += --gc-sections
release: ASFLAGS += -DRELEASE
release: clean all
	@printf "$(GREEN)🚀 Release build completed$(RESET)\n"
	@strip --strip-unneeded $(TARGET) 2>/dev/null || true

# Profile-guided optimization build
pgo-generate: CFLAGS += -fprofile-generate
pgo-generate: clean all
	@printf "$(YELLOW)📊 PGO instrumentation build completed$(RESET)\n"

pgo-use: CFLAGS += -fprofile-use -fprofile-correction
pgo-use: clean all
	@printf "$(GREEN)⚡ PGO optimized build completed$(RESET)\n"

# Install target (copy to boot directory)
install: $(TARGET)
	@printf "$(YELLOW)📦 Installing kernel...$(RESET)\n"
	@if [ -d "/boot" ]; then \
		sudo cp $(TARGET) /boot/hash_kernel.bin; \
		sudo chmod 644 /boot/hash_kernel.bin; \
		printf "$(GREEN)✅ Kernel installed to /boot/hash_kernel.bin$(RESET)\n"; \
	else \
		printf "$(RED)⚠️  /boot directory not found, skipping install$(RESET)\n"; \
	fi

# =============================================================================
# Testing and Validation
# =============================================================================

# Run static analysis
analyze: check-tools
	@printf "$(YELLOW)🔍 Running static analysis...$(RESET)\n"
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --inconclusive --std=c99 \
		         --suppress=missingIncludeSystem \
		         $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) 2>/dev/null || true; \
	fi
	@if command -v clang-tidy >/dev/null 2>&1; then \
		find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.c' \
		     -exec clang-tidy {} -- $(CFLAGS) \; 2>/dev/null || true; \
	fi
	@printf "$(GREEN)✅ Static analysis completed$(RESET)\n"

# Memory layout analysis
memory-map: $(TARGET_ELF)
	@printf "$(CYAN)🗺️  Memory Layout Analysis:$(RESET)\n"
	@$(OBJDUMP) -h $(TARGET_ELF) | grep -E '(LOAD|ALLOC)' || true
	@printf "$(CYAN)📊 Section sizes:$(RESET)\n"
	@size $(TARGET_ELF) 2>/dev/null || true

# Disassemble kernel for analysis
disasm: $(TARGET_ELF)
	@printf "$(CYAN)🔍 Generating disassembly...$(RESET)\n"
	$(OBJDUMP) -d $(TARGET_ELF) > $(BUILD_DIR)/kernel_disasm.txt
	@printf "$(GREEN)✅ Disassembly saved to $(BUILD_DIR)/kernel_disasm.txt$(RESET)\n"

# Test build in different configurations
test: clean
	@printf "$(YELLOW)🧪 Running build tests...$(RESET)\n"
	@$(MAKE) --no-print-directory clean debug >/dev/null 2>&1 && \
	 printf "$(GREEN)✅ Debug build test passed$(RESET)\n" || \
	 printf "$(RED)❌ Debug build test failed$(RESET)\n"
	@$(MAKE) --no-print-directory clean release >/dev/null 2>&1 && \
	 printf "$(GREEN)✅ Release build test passed$(RESET)\n" || \
	 printf "$(RED)❌ Release build test failed$(RESET)\n"
	@$(MAKE) --no-print-directory clean all >/dev/null 2>&1 && \
	 printf "$(GREEN)✅ Default build test passed$(RESET)\n" || \
	 printf "$(RED)❌ Default build test failed$(RESET)\n"

# =============================================================================
# Information and Help
# =============================================================================

# Show build information
info:
	@printf "$(BLUE)🏗️  HASH OS Kernel Build System$(RESET)\n"
	@printf "$(BLUE)================================$(RESET)\n"
	@printf "Build Date: $(CYAN)$(BUILD_DATE)$(RESET)\n"
	@printf "Version: $(CYAN)$(BUILD_VERSION)$(RESET)\n"
	@printf "Git Hash: $(CYAN)$(BUILD_HASH)$(RESET)\n"
	@printf "Target: $(CYAN)$(TARGET)$(RESET)\n"
	@printf "Compiler: $(CYAN)$(CC) $(shell $(CC) --version 2>/dev/null | head -1 || echo 'not found')$(RESET)\n"
	@printf "Assembler: $(CYAN)$(AS) $(shell $(AS) --version 2>/dev/null | head -1 || echo 'not found')$(RESET)\n"
	@printf "Parallel Jobs: $(CYAN)$(shell echo $(MAKEFLAGS) | grep -o 'jobs=[0-9]*' | cut -d= -f2 || echo 'sequential')$(RESET)\n"
	@printf "$(BLUE)================================$(RESET)\n"

# Show help
help:
	@printf "$(BLUE)HASH OS Kernel Build System$(RESET)\n"
	@printf "\n$(YELLOW)Available targets:$(RESET)\n"
	@printf "  $(GREEN)all$(RESET)       - Build the kernel (default)\n"
	@printf "  $(GREEN)debug$(RESET)     - Build debug version with symbols\n"
	@printf "  $(GREEN)release$(RESET)   - Build optimized release version\n"
	@printf "  $(GREEN)iso$(RESET)       - Create bootable ISO image\n"
	@printf "  $(GREEN)clean$(RESET)     - Remove build files\n"
	@printf "  $(GREEN)distclean$(RESET) - Remove all generated files\n"
	@printf "  $(GREEN)install$(RESET)   - Install kernel to /boot\n"
	@printf "  $(GREEN)test$(RESET)      - Run build tests\n"
	@printf "  $(GREEN)analyze$(RESET)   - Run static analysis\n"
	@printf "  $(GREEN)info$(RESET)      - Show build information\n"
	@printf "  $(GREEN)help$(RESET)      - Show this help message\n"
	@printf "  $(GREEN)deps$(RESET)      - Show file dependencies\n"
	@printf "  $(GREEN)stats$(RESET)     - Show code statistics\n"
	@printf "\n$(YELLOW)Advanced targets:$(RESET)\n"
	@printf "  $(GREEN)memory-map$(RESET) - Analyze memory layout\n"
	@printf "  $(GREEN)disasm$(RESET)     - Generate disassembly\n"
	@printf "  $(GREEN)pgo-generate$(RESET) - Build with PGO instrumentation\n"
	@printf "  $(GREEN)pgo-use$(RESET)    - Build with PGO optimization\n"
	@printf "\n$(YELLOW)Build configuration:$(RESET)\n"
	@printf "  CC=$(CYAN)$(CC)$(RESET) LD=$(CYAN)$(LD)$(RESET) AS=$(CYAN)$(AS)$(RESET)\n"

# Show dependencies
deps:
	@printf "$(CYAN)📋 File Dependencies:$(RESET)\n"
	@printf "Kernel sources: $(YELLOW)$(KERNEL_SOURCES) $(KERNEL_ASM_SOURCES)$(RESET)\n"
	@printf "Driver sources: $(YELLOW)$(DRIVER_SOURCES)$(RESET)\n"
	@printf "UI sources: $(YELLOW)$(UI_SOURCES)$(RESET)\n"
	@printf "Placeholder sources: $(YELLOW)$(PLACEHOLDER_SOURCES)$(RESET)\n"
	@printf "Total objects: $(CYAN)$(words $(ALL_OBJS))$(RESET)\n"

# =============================================================================
# Cleaning Targets
# =============================================================================

# Clean build files
clean:
	@printf "$(YELLOW)🧹 Cleaning build files...$(RESET)\n"
	$(RM) $(BUILD_DIR)
	$(RM) $(TARGET) $(TARGET_ELF) $(TARGET_ISO)
	$(RM) *.o *.elf *.bin *.map *.iso
	$(RM) iso/
	$(RM) *.gcda *.gcno # PGO files
	@printf "$(GREEN)✅ Build files cleaned$(RESET)\n"

# Complete clean including generated files
distclean: clean
	@printf "$(YELLOW)🧹 Deep cleaning...$(RESET)\n"
	$(RM) .depend
	$(RM) compile_commands.json
	$(RM) tags TAGS
	@printf "$(GREEN)✅ All files cleaned$(RESET)\n"

# =============================================================================
# Advanced Features
# =============================================================================

# Generate dependency files
.depend: $(wildcard $(KERNEL_DIR)/*.c $(DRIVERS_DIR)/*.c $(UI_DIR)/*.c *.c)
	@printf "$(CYAN)🔍 Generating dependencies...$(RESET)\n"
	@$(CC) $(CFLAGS) -MM $^ > .depend 2>/dev/null || true

# Include dependencies if they exist
-include .depend

# Generate compile_commands.json for IDEs
compile_commands.json: .depend
	@printf "$(CYAN)🔧 Generating compile commands...$(RESET)\n"
	@printf "[\n" > $@
	@for src in $(addprefix $(KERNEL_DIR)/,$(KERNEL_SOURCES)) \
	            $(addprefix $(DRIVERS_DIR)/,$(DRIVER_SOURCES)) \
	            $(addprefix $(UI_DIR)/,$(UI_SOURCES)) \
	            $(PLACEHOLDER_SOURCES); do \
		if [ -f "$$src" ]; then \
			printf "  {\n    \"directory\": \"%s\",\n    \"command\": \"%s %s -c %s\",\n    \"file\": \"%s\"\n  },\n" \
			       "$$(pwd)" "$(CC)" "$(CFLAGS)" "$$src" "$$src" >> $@; \
		fi; \
	done
	@sed -i '$$s/,$$//' $@ 2>/dev/null || sed -i '' '$$s/,$$//' $@ 2>/dev/null || true
	@printf "]\n" >> $@
	@printf "$(GREEN)✅ Compile commands generated$(RESET)\n"

# Check for required tools
check-tools:
	@printf "$(CYAN)🔧 Checking build tools...$(RESET)\n"
	@for tool in $(CC) $(AS) $(LD) $(OBJCOPY) $(OBJDUMP); do \
		if ! command -v $$tool >/dev/null 2>&1; then \
			printf "$(RED)❌ $$tool not found$(RESET)\n"; \
			exit 1; \
		fi; \
	done
	@printf "$(GREEN)✅ All required tools found$(RESET)\n"

# Validate source files exist
check-sources:
	@printf "$(CYAN)🔍 Validating source files...$(RESET)\n"
	@missing=0; \
	for file in $(addprefix $(KERNEL_DIR)/,$(KERNEL_SOURCES)) \
	            $(addprefix $(KERNEL_DIR)/,$(KERNEL_ASM_SOURCES)) \
	            $(addprefix $(DRIVERS_DIR)/,$(DRIVER_SOURCES)) \
	            $(addprefix $(UI_DIR)/,$(UI_SOURCES)) \
	            $(PLACEHOLDER_SOURCES); do \
		if [ ! -f "$$file" ]; then \
			printf "$(YELLOW)⚠️  Missing source file: $$file$(RESET)\n"; \
			missing=$$((missing + 1)); \
		fi; \
	done; \
	if [ $$missing -eq 0 ]; then \
		printf "$(GREEN)✅ All source files found$(RESET)\n"; \
	else \
		printf "$(YELLOW)⚠️  $$missing source files missing$(RESET)\n"; \
	fi

# Pre-build checks
pre-build: check-tools check-sources
	@printf "$(GREEN)✅ Pre-build checks passed$(RESET)\n"

# Full build with validation
build-safe: pre-build all

# =============================================================================
# Development Helpers
# =============================================================================

# Count lines of code
stats:
	@printf "$(CYAN)📊 Code Statistics:$(RESET)\n"
	@c_lines=$$(find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.c' -exec wc -l {} + 2>/dev/null | tail -1 | awk '{print $$1}' || echo '0'); \
	 asm_lines=$$(find $(KERNEL_DIR) -name '*.asm' -exec wc -l {} + 2>/dev/null | tail -1 | awk '{print $$1}' || echo '0'); \
	 header_files=$$(find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.h' | wc -l); \
	 total_files=$$(find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.c' -o -name '*.h' -o -name '*.asm' | wc -l); \
	 printf "Lines of C code: $(YELLOW)$$c_lines$(RESET)\n"; \
	 printf "Lines of Assembly: $(YELLOW)$$asm_lines$(RESET)\n"; \
	 printf "Header files: $(YELLOW)$$header_files$(RESET)\n"; \
	 printf "Total files: $(YELLOW)$$total_files$(RESET)\n"

# Quick syntax check without building
syntax-check:
	@printf "$(CYAN)🔍 Checking syntax...$(RESET)\n"
	@errors=0; \
	for file in $(addprefix $(KERNEL_DIR)/,$(KERNEL_SOURCES)) \
	            $(addprefix $(DRIVERS_DIR)/,$(DRIVER_SOURCES)) \
	            $(addprefix $(UI_DIR)/,$(UI_SOURCES)) \
	            $(PLACEHOLDER_SOURCES); do \
		if [ -f "$$file" ]; then \
			if ! $(CC) $(CFLAGS) -fsyntax-only "$$file" 2>/dev/null; then \
				printf "$(RED)❌ Syntax error in $$file$(RESET)\n"; \
				errors=$$((errors + 1)); \
			fi; \
		fi; \
	done; \
	if [ $$errors -eq 0 ]; then \
		printf "$(GREEN)✅ Syntax check passed$(RESET)\n"; \
	else \
		printf "$(RED)❌ $$errors syntax errors found$(RESET)\n"; \
		exit 1; \
	fi

# Generate tags for code navigation
tags:
	@printf "$(CYAN)🏷️  Generating tags...$(RESET)\n"
	@if command -v ctags >/dev/null 2>&1; then \
		ctags -R $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR); \
		printf "$(GREEN)✅ Tags generated$(RESET)\n"; \
	else \
		printf "$(YELLOW)⚠️  ctags not found$(RESET)\n"; \
	fi

# Watch for changes and rebuild
watch:
	@if command -v inotifywait >/dev/null 2>&1; then \
		printf "$(CYAN)👀 Watching for changes...$(RESET)\n"; \
		while true; do \
			inotifywait -r -e modify $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) 2>/dev/null; \
			printf "$(YELLOW)🔄 Files changed, rebuilding...$(RESET)\n"; \
			$(MAKE) --no-print-directory all; \
		done; \
	else \
		printf "$(RED)❌ inotifywait not found$(RESET)\n"; \
	fi

# =============================================================================
# Error Handling and Quality Assurance
# =============================================================================

# Make sure we stop on errors
.DELETE_ON_ERROR:

# Default shell options
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

# Phony targets to avoid conflicts
.PHONY: all clean distclean info help debug release install iso test analyze \
        memory-map disasm pgo-generate pgo-use check-tools check-sources \
        pre-build build-safe stats syntax-check tags watch compile_commands.json
