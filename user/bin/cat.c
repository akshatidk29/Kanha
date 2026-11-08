#include "stdio.h"
#include "ulib.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf(COLOR_CYAN "Usage: cat <filename>\n" COLOR_RESET);
        return 1;
    }

    int fd = open(argv[1], 0);
    if (fd < 0) {
        printf(COLOR_BRIGHT_RED "cat: cannot open %s\n" COLOR_RESET, argv[1]);
        return 1;
    }

    char buffer[256];
    int bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    close(fd);
    return 0;
}

