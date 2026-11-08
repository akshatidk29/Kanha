#include "stdio.h"
#include "ulib.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_CYAN "Usage: mkdir <dirname>" COLOR_RESET "\n");
        return 1;
    }

    if (mkdir(argv[1]) != 0) {
        printf(COLOR_BRIGHT_RED "mkdir: cannot create directory %s" COLOR_RESET "\n", argv[1]);
        return 1;
    }

    return 0;
}

