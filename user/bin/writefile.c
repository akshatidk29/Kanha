#include "stdio.h"
#include "ulib.h"
#include "string.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf(COLOR_CYAN "Usage: writefile <filename> <content>" COLOR_RESET "\n");
        return 1;
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY);
    if (fd < 0) {
        printf(COLOR_BRIGHT_RED "writefile: cannot open %s" COLOR_RESET "\n", argv[1]);
        return 1;
    }

    for (int i = 2; i < argc; i++) {
        write(fd, argv[i], strlen(argv[i]));
        if (i < argc - 1) {
            write(fd, " ", 1);
        }
    }
    write(fd, "\n", 1);

    close(fd);
    return 0;
}

