#include "stdio.h"
#include "ulib.h"

int main(void) {
    printf("User Mode!\n");
    printf("Type your name: ");
    char name[32];
    getline(name, sizeof(name));
    printf("\nWelcome, %s!\n", name);
    return 0;
}
