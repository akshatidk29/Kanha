cmake_minimum_required(VERSION 3.20)

project(Kanha
        VERSION 0.1.0
        LANGUAGES C ASM_NASM 
)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_FLAGS "-f elf32")
set(CMAKE_C_FLAGS "-m32 -ffreestanding -g -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-m,elf_i386 -Wl,-T,${CMAKE_SOURCE_DIR}/linker/linker.ld -Wl,--oformat=elf32-i386")


add_executable(kernel.elf
    kernel/arch/x86/boot/start.asm
    kernel/core/kernel.c
    kernel/drivers/vga/vga.c
    kernel/drivers/mouse/mouse.c
    kernel/drivers/keyboard/keyboard.c
    kernel/arch/x86/hardware/pic.c
    kernel/arch/x86/interrupts/isr.asm
    kernel/arch/x86/interrupts/idt.c
    kernel/arch/x86/interrupts/exception.c
    kernel/arch/x86/hardware/timer.c
)

target_include_directories(kernel.elf 
    PRIVATE
    kernel
    kernel/arch/x86
    kernel/arch/x86/interrupts
    kernel/arch/x86/hardware
    kernel/drivers/vga
    kernel/drivers/keyboard
    kernel/drivers/mouse
)

set(BUILD_DIR ${CMAKE_BINARY_DIR})

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
    COMMAND qemu-system-i386 -drive file=${BUILD_DIR}/kanha.img,format=raw
    DEPENDS kanha.img
    COMMENT "Running Kanha"
)

# Debug
add_custom_target(debug
    COMMAND qemu-system-i386
            -drive file=${BUILD_DIR}/kanha.img,format=raw
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