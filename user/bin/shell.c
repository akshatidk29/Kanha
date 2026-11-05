#include "stdio.h"
#include "ulib.h"
#include "string.h"

int main(void) {  

    while(1){
        printf("Kanha OS> ");
        char cmd[32];
        getline(cmd, sizeof(cmd));
        printf("You Entered: %s\n", cmd);

        if(strcmp(cmd, "run init") == 0){
            int id = fork();
            if(id == 0){
                int ret = exec("/bin/init");
                if(ret == -1){
                    printf("Failed to execute /bin/init\n");
                }
            }else{
                sleep(5000);
            }
        }else if(strcmp(cmd, "ls") == 0){
            char buffer[100];
            readDir("/bin", buffer, 100);
            printf("%s\n", buffer);
        }
    } 
    
    return 0;
}
