#include "stdio.h"
#include "ulib.h"
#include "string.h"

int main(int argc, char **argv) {
    
    char buffer[256];
    const char *path = "/";

    if (argc > 1 && argv[1] != NULL && argv[1][0] != '\0') {
        path = argv[1];
    }

    int count = readDir(path, buffer, sizeof(buffer));
    if (count < 0) {
        printf("Directory not found: %s\n", path);
        return 1;
    } else if (count > 0) {
        printf("%s", buffer);
    }

    return 0;
}
