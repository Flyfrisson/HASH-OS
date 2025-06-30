# HASH OS Kernel Makefile - Enhanced with Error Handling and Optimization
# =============================================================================

# Build tools
AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
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
         -Wall -Wextra -Werror -O2 -g $(INCLUDES)

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

# Build configuration
BUILD_DATE = $(shell date '+%Y-%m-%d %H:%M:%S')
BUILD_VERSION = $(shell git describe --tags --always 2>/dev/null || echo "unknown")

# =============================================================================
# Main Targets
# =============================================================================

.PHONY: all clean distclean info help debug release install

# Default target
all: info $(TARGET)
	@echo "✅ Build complete: $(TARGET) is ready."
	@echo "📊 Build size: $$(du -h $(TARGET) | cut -f1)"

# Create kernel binary
$(TARGET): $(BUILD_DIR) $(ALL_OBJS) linker.ld
	@echo "🔗 Linking kernel..."
	$(LD) $(LDFLAGS) -o $(TARGET) $(ALL_OBJS)
	@echo "📋 Kernel memory layout:"
	@$(LD) $(LDFLAGS) --print-map -o $(TARGET_ELF) $(ALL_OBJS) | head -20 || true

# Create build directory
$(BUILD_DIR):
	@echo "📁 Creating build directory..."
	$(MKDIR) $(BUILD_DIR)

# =============================================================================
# Build Rules for C Files
# =============================================================================

# Kernel C files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@echo "🔨 Compiling kernel: $<"
	$(CC) $(CFLAGS) -DBUILD_DATE="\"$(BUILD_DATE)\"" -DBUILD_VERSION="\"$(BUILD_VERSION)\"" -c $< -o $@

# Driver C files
$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c
	@echo "🔨 Compiling driver: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# UI C files
$(BUILD_DIR)/%.o: $(UI_DIR)/%.c
	@echo "🔨 Compiling UI: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Placeholder files (root directory)
$(BUILD_DIR)/%.o: %.c
	@echo "🔨 Compiling placeholder: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# =============================================================================
# Build Rules for Assembly Files
# =============================================================================

# Kernel assembly files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	@echo "🔧 Assembling: $<"
	$(AS) $(ASFLAGS) $< -o $@

# =============================================================================
# Special Targets
# =============================================================================

# Debug build with extra symbols and no optimization
debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: ASFLAGS += -DDEBUG
debug: clean all
	@echo "🐛 Debug build completed"

# Release build with maximum optimization
release: CFLAGS += -DRELEASE -O3 -DNDEBUG
release: ASFLAGS += -DRELEASE
release: clean all
	@echo "🚀 Release build completed"

# Install target (copy to boot directory)
install: $(TARGET)
	@echo "📦 Installing kernel..."
	@if [ -d "/boot" ]; then \
		cp $(TARGET) /boot/hash_kernel.bin; \
		echo "✅ Kernel installed to /boot/hash_kernel.bin"; \
	else \
		echo "⚠️  /boot directory not found, skipping install"; \
	fi

# =============================================================================
# Information and Help
# =============================================================================

# Show build information
info:
	@echo "🏗️  HASH OS Kernel Build System"
	@echo "================================"
	@echo "Build Date: $(BUILD_DATE)"
	@echo "Version: $(BUILD_VERSION)"
	@echo "Target: $(TARGET)"
	@echo "Compiler: $(CC) $(shell $(CC) --version 2>/dev/null | head -1 || echo 'not found')"
	@echo "Assembler: $(AS) $(shell $(AS) --version 2>/dev/null | head -1 || echo 'not found')"
	@echo "================================"

# Show help
help:
	@echo "HASH OS Kernel Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all      - Build the kernel (default)"
	@echo "  debug    - Build debug version with symbols"
	@echo "  release  - Build optimized release version"
	@echo "  clean    - Remove build files"
	@echo "  distclean- Remove all generated files"
	@echo "  install  - Install kernel to /boot"
	@echo "  info     - Show build information"
	@echo "  help     - Show this help message"
	@echo "  deps     - Show file dependencies"
	@echo ""
	@echo "Build configuration:"
	@echo "  CC=$(CC) LD=$(LD) AS=$(AS)"
	@echo "  CFLAGS=$(CFLAGS)"

# Show dependencies
deps:
	@echo "📋 File Dependencies:"
	@echo "Kernel sources: $(KERNEL_SOURCES) $(KERNEL_ASM_SOURCES)"
	@echo "Driver sources: $(DRIVER_SOURCES)"
	@echo "UI sources: $(UI_SOURCES)"
	@echo "Placeholder sources: $(PLACEHOLDER_SOURCES)"
	@echo "Total objects: $(words $(ALL_OBJS))"

# =============================================================================
# Cleaning Targets
# =============================================================================

# Clean build files
clean:
	@echo "🧹 Cleaning build files..."
	$(RM) $(BUILD_DIR)
	$(RM) $(TARGET) $(TARGET_ELF)
	$(RM) *.o *.elf *.bin *.map
	@echo "✅ Build files cleaned"

# Complete clean including generated files
distclean: clean
	@echo "🧹 Deep cleaning..."
	$(RM) .depend
	$(RM) compile_commands.json
	@echo "✅ All files cleaned"

# =============================================================================
# Advanced Features
# =============================================================================

# Generate dependency files
.depend: $(wildcard $(KERNEL_DIR)/*.c $(DRIVERS_DIR)/*.c $(UI_DIR)/*.c *.c)
	@echo "🔍 Generating dependencies..."
	@$(CC) $(CFLAGS) -MM $^ > .depend 2>/dev/null || true

# Include dependencies if they exist
-include .depend

# Check for required tools
check-tools:
	@echo "🔧 Checking build tools..."
	@which $(CC) >/dev/null || (echo "❌ $(CC) not found" && exit 1)
	@which $(AS) >/dev/null || (echo "❌ $(AS) not found" && exit 1)
	@which $(LD) >/dev/null || (echo "❌ $(LD) not found" && exit 1)
	@which $(OBJCOPY) >/dev/null || (echo "❌ $(OBJCOPY) not found" && exit 1)
	@echo "✅ All tools found"

# Validate source files exist
check-sources:
	@echo "🔍 Validating source files..."
	@for file in $(addprefix $(KERNEL_DIR)/,$(KERNEL_SOURCES)) \
	             $(addprefix $(KERNEL_DIR)/,$(KERNEL_ASM_SOURCES)) \
	             $(addprefix $(DRIVERS_DIR)/,$(DRIVER_SOURCES)) \
	             $(addprefix $(UI_DIR)/,$(UI_SOURCES)) \
	             $(PLACEHOLDER_SOURCES); do \
		if [ ! -f "$$file" ]; then \
			echo "⚠️  Missing source file: $$file"; \
		fi; \
	done
	@echo "✅ Source validation complete"

# Pre-build checks
pre-build: check-tools check-sources
	@echo "✅ Pre-build checks passed"

# Full build with validation
build-safe: pre-build all

# =============================================================================
# Development Helpers
# =============================================================================

# Count lines of code
stats:
	@echo "📊 Code Statistics:"
	@echo "Lines of C code: $$(find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.c' -exec wc -l {} + 2>/dev/null | tail -1 | awk '{print $$1}' || echo '0')"
	@echo "Lines of Assembly: $$(find $(KERNEL_DIR) -name '*.asm' -exec wc -l {} + 2>/dev/null | tail -1 | awk '{print $$1}' || echo '0')"
	@echo "Header files: $$(find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.h' | wc -l)"
	@echo "Total files: $$(find $(KERNEL_DIR) $(DRIVERS_DIR) $(UI_DIR) -name '*.c' -o -name '*.h' -o -name '*.asm' | wc -l)"

# Quick syntax check without building
syntax-check:
	@echo "🔍 Checking syntax..."
	@for file in $(addprefix $(KERNEL_DIR)/,$(KERNEL_SOURCES)) \
	             $(addprefix $(DRIVERS_DIR)/,$(DRIVER_SOURCES)) \
	             $(addprefix $(UI_DIR)/,$(UI_SOURCES)) \
	             $(PLACEHOLDER_SOURCES); do \
		if [ -f "$$file" ]; then \
			$(CC) $(CFLAGS) -fsyntax-only "$$file" || exit 1; \
		fi; \
	done
	@echo "✅ Syntax check passed"

# =============================================================================
# Error Handling
# =============================================================================

# Make sure we stop on errors
.DELETE_ON_ERROR:

# Default shell options
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c