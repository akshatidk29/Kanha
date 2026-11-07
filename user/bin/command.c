#include "stdio.h"
#include "ulib.h"

int main(void) {
    char* argv[3];
    argv[0] = "ls";
    argv[1] = "/bin";
    argv[2] = NULL;
    exec("/bin/ls", argv);
    return 0;
}
