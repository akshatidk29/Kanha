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

# Compile common user runtime libraries 
add_custom_command(
    OUTPUT ${BUILD_DIR}/userCrt0.o
    COMMAND nasm -f elf32 ${CMAKE_SOURCE_DIR}/user/lib/crt0.asm -o ${BUILD_DIR}/userCrt0.o
    DEPENDS ${CMAKE_SOURCE_DIR}/user/lib/crt0.asm
    COMMENT "Building user crt0.o"
)

add_custom_command(
    OUTPUT ${BUILD_DIR}/userStdio.o
    COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/lib/stdio.c -o ${BUILD_DIR}/userStdio.o
    DEPENDS ${CMAKE_SOURCE_DIR}/user/lib/stdio.c ${CMAKE_SOURCE_DIR}/user/lib/stdio.h
    COMMENT "Building user stdio.o"
)

add_custom_command(
    OUTPUT ${BUILD_DIR}/userUlib.o
    COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/lib/ulib.c -o ${BUILD_DIR}/userUlib.o
    DEPENDS ${CMAKE_SOURCE_DIR}/user/lib/ulib.c ${CMAKE_SOURCE_DIR}/user/lib/ulib.h
    COMMENT "Building user ulib.o"
)

add_custom_command(
    OUTPUT ${BUILD_DIR}/userString.o
    COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/lib/string.c -o ${BUILD_DIR}/userString.o
    DEPENDS ${CMAKE_SOURCE_DIR}/user/lib/string.c ${CMAKE_SOURCE_DIR}/user/lib/string.h
    COMMENT "Building user string.o"
)

set(USER_COMMON_OBJS
    ${BUILD_DIR}/userCrt0.o
    ${BUILD_DIR}/userStdio.o
    ${BUILD_DIR}/userUlib.o
    ${BUILD_DIR}/userString.o
)

macro(build_user_program name)
    add_custom_command(
        OUTPUT ${BUILD_DIR}/bin/${name}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_DIR}/bin
        COMMAND gcc -m32 -ffreestanding -g -fno-pic -fno-pie -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I${CMAKE_SOURCE_DIR}/user/lib -I${CMAKE_SOURCE_DIR}/kernel/syscall -c ${CMAKE_SOURCE_DIR}/user/bin/${name}.c -o ${BUILD_DIR}/${name}.o
        COMMAND ld -m elf_i386 -T ${CMAKE_SOURCE_DIR}/user/lib/user.ld -no-pie ${USER_COMMON_OBJS} ${BUILD_DIR}/${name}.o -o ${BUILD_DIR}/${name}.elf
        COMMAND objcopy -O binary ${BUILD_DIR}/${name}.elf ${BUILD_DIR}/bin/${name}
        DEPENDS ${CMAKE_SOURCE_DIR}/user/bin/${name}.c ${USER_COMMON_OBJS} ${CMAKE_SOURCE_DIR}/user/lib/user.ld
        COMMENT "Building user program ${name}"
    )
    list(APPEND USER_PROGRAMS ${BUILD_DIR}/bin/${name})
endmacro()

# User programs compilation
build_user_program(help)
build_user_program(shell)
build_user_program(ls)
build_user_program(cat)
build_user_program(touch)
build_user_program(clear)
build_user_program(mkdir)
build_user_program(writefile)

set(DISK_IMAGE ${CMAKE_SOURCE_DIR}/disk.img)

add_custom_target(populateDisk ALL
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/populateDisk.py ${DISK_IMAGE} ${BUILD_DIR}/bin
    DEPENDS ${USER_PROGRAMS}
    COMMENT "Installing user binaries into disk.img (/bin)"
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
    kernel/drivers/ata.c
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
    kernel/lib/string.c
    kernel/task/lock.c
    kernel/task/thread.c
    kernel/task/switch.asm
    kernel/task/scheduler.c
    kernel/task/process.c
    kernel/syscall/switch.asm
    kernel/syscall/syscall.c
    kernel/fs/vfs.c
    kernel/fs/bcache.c
    kernel/fs/ext2.c
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
    DEPENDS kernel.elf ${BUILD_DIR}/boot.bin populateDisk
    COMMENT "Building Kanha"
)

# Run 
add_custom_target(run
    COMMAND qemu-system-i386 -m 512M -drive file=${BUILD_DIR}/kanha.img,format=raw,index=0,media=disk -drive file=${DISK_IMAGE},format=raw,index=1,media=disk -global i8042.kbd-throttle=on -no-reboot -serial stdio
    DEPENDS kanha.img
    COMMENT "Running Kanha"
)

# Run-nox 
add_custom_target(run-nox  
    COMMAND clear
    COMMAND qemu-system-i386 -m 512M -drive file=${BUILD_DIR}/kanha.img,format=raw,index=0,media=disk -drive file=${DISK_IMAGE},format=raw,index=1,media=disk -global i8042.kbd-throttle=on -no-reboot -nographic -serial mon:stdio
    DEPENDS kanha.img
    COMMENT "Running Kanha in terminal (no GUI)"
)
                   
# Debug
add_custom_target(debug
    COMMAND qemu-system-i386
            -m 512M
            -drive file=${BUILD_DIR}/kanha.img,format=raw,index=0,media=disk
            -drive file=${DISK_IMAGE},format=raw,index=1,media=disk
            -global i8042.kbd-throttle=on
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