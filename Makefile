CC = gcc
AS = nasm
LD = ld
PYTHON = python3
OBJCOPY = objcopy

BUILD = build
TOOLS = tools

CFLAGS = -m32 \
         -ffreestanding \
         -fno-pie \
         -fno-pic \
         -fno-stack-protector \
         -fno-unwind-tables \
         -fno-asynchronous-unwind-tables

all: $(BUILD)/kanha.img


# Bootloader
$(BUILD)/boot.bin: boot/boot.asm
	$(AS) -f bin $< -o $@


# Kernel Entry
$(BUILD)/start.o: kernel/arch/x86/start.asm
	$(AS) -f elf32 $< -o $@


# C Kernel
$(BUILD)/kernel.o: kernel/core/kernel.c 
	$(CC) $(CFLAGS) -c $< -o $@

# VGA Driver
$(BUILD)/vga.o: kernel/drivers/vga/vga.c
	$(CC) $(CFLAGS) -Ikernel -c $< -o $@




# Link Kernel
$(BUILD)/kernel.elf: $(BUILD)/start.o $(BUILD)/kernel.o $(BUILD)/vga.o linker/linker.ld
	$(LD) -m elf_i386 \
	      -T linker/linker.ld \
	      -o $@ \
	      $(BUILD)/start.o \
	      $(BUILD)/kernel.o \
	      $(BUILD)/vga.o


# ELF -> Binary
$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	$(OBJCOPY) -O binary $< $@

# Make Header
$(BUILD)/header.bin: $(BUILD)/kernel.bin
	$(PYTHON) $(TOOLS)/makeHeader.py $(BUILD)/kernel.bin $(BUILD)/header.bin

# Make Image
$(BUILD)/kanha.img: $(BUILD)/boot.bin $(BUILD)/header.bin $(BUILD)/kernel.bin tools/makeImage.py
	$(PYTHON) tools/makeImage.py \
		$(BUILD)/boot.bin \
		$(BUILD)/header.bin \
		$(BUILD)/kernel.bin \
		$@

# Run
run: $(BUILD)/kanha.img
	qemu-system-x86_64 \
		-drive format=raw,file=$(BUILD)/kanha.img


# Clean
clean:
	rm -rf $(BUILD)/* 