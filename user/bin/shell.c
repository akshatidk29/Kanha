#include "stdio.h"
#include "ulib.h"

int main(void) {  

    while(1){
        printf("Kanha OS> ");
        char cmd[32];
        getline(cmd, sizeof(cmd));
        printf("%s\n", cmd);
    } 
    
    return 0;
}
