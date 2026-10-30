#include "stdio.h"
#include "ulib.h"

int main(void) {
    printf("Hello from Kanha OS C User Mode!\n");
    printf("Type your name: ");
    char name[32];
    getline(name, sizeof(name));
    printf("\nWelcome, %s! User mode is working!\n", name);
    return 0;
}
