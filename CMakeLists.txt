cmake_minimum_required(VERSION 3.20)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)

project(Kanha
        VERSION 0.1.0
        LANGUAGES C ASM_NASM 
)

set(CMAKE_ASM_NASM_FLAGS "-f elf32")
set(CMAKE_C_FLAGS "-m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables")
set(CMAKE_EXE_LINKER_FLAGS "-no-pie -nostdlib -Wl,-m,elf_i386 -Wl,-T,${CMAKE_SOURCE_DIR}/linker/linker.ld -Wl,--oformat=elf32-i386")

set(BUILD_DIR ${CMAKE_BINARY_DIR})

add_custom_command(
    OUTPUT ${BUILD_DIR}/init.bin
    COMMAND nasm -f elf32 ${CMAKE_SOURCE_DIR}/user/lib/crt0.asm -o ${BUILD_DIR}/crt0.o
    COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/bin/init.c -o ${BUILD_DIR}/init.o
    COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/lib/stdio.c -o ${BUILD_DIR}/stdio.o
    COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/lib/ulib.c -o ${BUILD_DIR}/ulib.o
    COMMAND ld -m elf_i386 -T ${CMAKE_SOURCE_DIR}/user/lib/user.ld -no-pie ${BUILD_DIR}/crt0.o ${BUILD_DIR}/init.o ${BUILD_DIR}/stdio.o ${BUILD_DIR}/ulib.o -o ${BUILD_DIR}/init.elf
    COMMAND objcopy -O binary ${BUILD_DIR}/init.elf ${BUILD_DIR}/init.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/user/lib/crt0.asm ${CMAKE_SOURCE_DIR}/user/bin/init.c ${CMAKE_SOURCE_DIR}/user/lib/stdio.c ${CMAKE_SOURCE_DIR}/user/lib/ulib.c ${CMAKE_SOURCE_DIR}/user/lib/user.ld
    COMMENT "Building user init.bin"
)

add_custom_command(
    OUTPUT ${BUILD_DIR}/initBin.o
    COMMAND ld -m elf_i386 -r -b binary -o ${BUILD_DIR}/initBin.o init.bin
    WORKING_DIRECTORY ${BUILD_DIR}
    DEPENDS ${BUILD_DIR}/init.bin
    COMMENT "Converting init.bin to initBin.o"
)

add_executable(kernel.elf
    kernel/arch/x86/boot/start.asm
    kernel/core/kernel.c
    kernel/lib/math.c
    kernel/mm/pmm.c
    kernel/drivers/vga.c
    kernel/drivers/mouse.c
    kernel/drivers/keyboard.c
    kernel/drivers/stdin.c
    kernel/drivers/serial.c
    kernel/arch/x86/hardware/pic.c
    kernel/arch/x86/hardware/timer.c
    kernel/arch/x86/hardware/tss.c
    kernel/arch/x86/hardware/paging.asm
    kernel/arch/x86/interrupts/isr.asm
    kernel/arch/x86/interrupts/idt.c
    kernel/arch/x86/interrupts/exception.c
    kernel/mm/paging.c
    kernel/mm/addressSpace.c
    kernel/mm/heap.c
    kernel/lib/kprintf.c
    kernel/task/lock.c
    kernel/task/thread.c
    kernel/task/switch.asm
    kernel/task/scheduler.c
    kernel/task/process.c
    kernel/syscall/switch.asm
    kernel/syscall/syscall.c
    ${BUILD_DIR}/initBin.o
)

target_include_directories(kernel.elf 
    PRIVATE
    kernel
    kernel/arch/x86
)

add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND objcopy -O binary $<TARGET_FILE:kernel.elf> ${BUILD_DIR}/kernel.bin
    COMMENT "Converting kernel.elf to kernel.bin"
)

add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/makeHeader.py
                    ${BUILD_DIR}/kernel.bin
                    ${BUILD_DIR}/header.bin
    COMMENT "Generating header.bin"
)


add_custom_command(OUTPUT ${BUILD_DIR}/boot.bin
    COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/boot/boot.asm -o ${BUILD_DIR}/boot.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/boot/boot.asm
)

# Final image
add_custom_target(kanha.img ALL
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/makeImage.py
                    ${BUILD_DIR}/boot.bin
                    ${BUILD_DIR}/header.bin
                    ${BUILD_DIR}/kernel.bin
                    ${BUILD_DIR}/kanha.img
    DEPENDS kernel.elf ${BUILD_DIR}/boot.bin
    COMMENT "Building Kanha"
)



# Run 
add_custom_target(run
    COMMAND qemu-system-i386 -m 512M -drive file=${BUILD_DIR}/kanha.img,format=raw -no-reboot -serial stdio
    DEPENDS kanha.img
    COMMENT "Running Kanha"
)

# Debug
add_custom_target(debug
    COMMAND qemu-system-i386
            -m 512M
            -drive file=${BUILD_DIR}/kanha.img,format=raw
            -serial stdio
            -S
            -gdb tcp::1234
    DEPENDS kanha.img
    COMMENT "Debugging Kanha on port 1234"
)

# Clear
add_custom_target(clear
    COMMAND cmake --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Cleaning build files"
)