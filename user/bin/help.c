#include "stdio.h"
#include "ulib.h"

int main(void) {
    printf(COLOR_YELLOW "Available Commands\n" COLOR_RESET);
    char *argv[3];
    argv[0] = "ls";
    argv[1] = "/bin";
    argv[2] = NULL;
    exec("/bin/ls", argv);

    return 0;
}
