#include "stdio.h"
#include "ulib.h"

int main(void) {
    printf("Type your name: ");
    char name[32];
    getline(name, sizeof(name));
    printf("Hello, %s!\n", name);
    return 0;
}
