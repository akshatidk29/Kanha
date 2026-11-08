#include "stdio.h"
#include "ulib.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf(COLOR_CYAN "Usage: touch <filename>\n" COLOR_RESET);
        return 1;
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY);
    if (fd < 0) {
        printf(COLOR_BRIGHT_RED "touch: cannot create %s" COLOR_RESET "\n", argv[1]);
        return 1;
    }

    close(fd);
    return 0;
}

