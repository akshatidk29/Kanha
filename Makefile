CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

BUILD = build

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


# Kernel entry
$(BUILD)/start.o: kernel/arch/x86/start.asm
	$(AS) -f elf32 $< -o $@


# C kernel
$(BUILD)/kernel.o: kernel/core/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@


# Link kernel
$(BUILD)/kernel.elf: $(BUILD)/start.o $(BUILD)/kernel.o linker/linker.ld
	$(LD) -m elf_i386 \
	      -T linker/linker.ld \
	      -o $@ \
	      $(BUILD)/start.o \
	      $(BUILD)/kernel.o


# ELF -> Binary
$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	$(OBJCOPY) -O binary $< $@


# Disk image
$(BUILD)/kanha.img: $(BUILD)/boot.bin $(BUILD)/kernel.bin
	cat $(BUILD)/boot.bin $(BUILD)/kernel.bin > $@


# Run
run: $(BUILD)/kanha.img
	qemu-system-x86_64 \
		-drive format=raw,file=$(BUILD)/kanha.img


# Clean
clean:
	rm -rf $(BUILD)/* 