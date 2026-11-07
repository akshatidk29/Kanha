# Kanha

Kanha is a 32-bit x86 monolithic operating system kernel written from scratch in C and NASM, inspired by xv6. It implements a higher-half kernel, virtual memory paging, preemptive multitasking, a virtual filesystem, and POSIX-style process management.

## Features

### Architecture and Booting
* Target architecture: 32-bit x86.
* Custom two-stage bootloader with BIOS E820 memory map parsing.
* Higher-half kernel mapped at `0xC0000000` (3 GB user / 1 GB kernel split).
* 512 MB physical memory mapped into the kernel address space.
* Interrupt handling via custom IDT, PIC, and PIT timer.
* Ring 3 to Ring 0 privilege level transitions.

### Memory Management
* **Physical Memory Manager (PMM)**: Bitmap allocator tracking available physical frames.
* **Virtual Memory**: Two-level paging (Page Directory and Page Tables) providing private address spaces for each process.
* **Kernel Heap**: Dynamic allocator (`kMalloc`, `kFree`) supporting block splitting and coalescing.
* Will implement demand paging soon.

### Tasking and Scheduling
* **Preemptive Multitasking**: Round-robin scheduler along with timer interrupts.
* **Kernel Threads**: Primary execution units with private kernel stacks and cooperative/preemptive context switching.
* **Process Model**: Each process contains an address space, PID, and a list of threads.
* **POSIX Process Primitives**:
  * `fork()`: Duplicates caller address space and registers for child execution.
  * `exec()`: Replaces process image, copies arguments into kernel memory, and builds user stack with `argc` and `argv`.
  * `waitpid()`: Blocks until child process terminates, retrieves exit code, and reaps memory.
* Process termination exits all sibling threads.

### Virtual File System (VFS)
* **Tree-based VFS**: Hierarchical structure implemented using LCRS linked nodes.
* **TarFS**: Reads an embedded `initrd.tar` ramdisk and builds the `/bin` directory tree at boot.

### System Calls
* `SYS_EXIT`     : Terminate process/thread and return status code.
* `SYS_WRITE`: Output to VGA console and COM1 serial port.
* `SYS_YIELD`: Cooperative CPU yield.
* `SYS_SLEEP`: Sleep for a given duration in milliseconds.
* `SYS_READ`: Read input from keyboard / stdin buffer.
* `SYS_SPAWN`: Direct binary spawn.
* `SYS_READDIR`: List directory entries.
* `SYS_FORK`: Clone process.
* `SYS_EXEC`: Execute program with arguments.
* `SYS_WAITPID` : Wait for child process termination and reap resources.

### User Space
* Custom C runtime startup (`crt0.asm`) passing `argc` and `argv` to `main(int argc, char **argv)`.
* User-level library (`ulib.h`, `stdio.h`, `string.h`).
* Included binaries:
  * `/bin/shell`: Interactive shell with argument parsing, `/bin/` path resolution, and `fork`/`exec`/`waitpid` workflow.
  * `/bin/ls`: Directory listing utility supporting path arguments.
  * `/bin/init`: Interactive user program.

## Building and Running

### Prerequisites
* `gcc-multilib`
* `nasm`
* `cmake`
* `python3`
* `qemu-system-i386`

### Build
```bash
cmake -B build
cmake --build build
```

### Run
```bash
qemu-system-i386 -m 512M -drive file=build/kanha.img,format=raw -serial stdio
```
