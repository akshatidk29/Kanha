#include "stdio.h"
#include "ulib.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cat <filename>\n");
        return 1;
    }

    int fd = open(argv[1], 0);
    if (fd < 0) {
        printf("cat: cannot open %s\n", argv[1]);
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

