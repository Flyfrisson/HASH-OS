#!/bin/bash
set -e

echo "🔨 Building HASH OS Bootloader..."
cd bootloader
nasm -f bin bootloader.asm -o bootloader.bin
cd ..

echo "🔨 Building HASH OS Kernel..."
cd kernel
make clean
make all
cd ..

echo "💾 Creating bootable floppy image..."
dd if=/dev/zero of=bootloader.img bs=512 count=2880
dd if=bootloader/bootloader.bin of=bootloader.img conv=notrunc
dd if=kernel/kernel.bin of=bootloader.img bs=512 seek=1 conv=notrunc

echo "🚀 Launching HASH OS in QEMU..."
qemu-system-x86_64 -fda bootloader.img
