#include "stdio.h"
#include "ulib.h"
#include "string.h"

#define MAX_CMD 64

int main(void) {  

    while(1){

        printf("Kanha OS> ");

        char input[MAX_CMD];
        getline(input, sizeof(input));

        char* cmd = input;

        while(cmd[0] == ' '){
            cmd = cmd + 1;
        }

        if(cmd[0] == '\0'){
            continue;
        }

        if(strcmp(cmd, "run init") == 0){
            int id = fork();
            if(id == 0){
                int ret = exec("/bin/init");
                if(ret == -1)   
                    printf("Failed to execute /bin/init\n");
            }else{
                int status = 0;
                waitpid(id, &status, 0);
            }
        }else if(strcmp(cmd, "ls") == 0 || strncmp(cmd, "ls ", 3) == 0){
            
            char buffer[100];
            char *path = "/";
            uint32_t i = 2;

            while(i < MAX_CMD && cmd[i] == ' ')
                i++;

            if(i < MAX_CMD && cmd[i] != '\0'){
                path = cmd + i;
                size_t plen = strlen(path);
                while(plen > 0 && path[plen - 1] == ' '){
                    path[plen - 1] = '\0';
                    plen--;
                }
            }

            int count = readDir(path, buffer, sizeof(buffer));
            if(count < 0){
                printf("Directory not found: %s\n", path);
            }else if(count > 0){
                printf("%s", buffer);
            }
        }else if(strcmp(cmd, "exit") == 0){
            break;
        }else{
            printf("Unknown Command: %s\n", cmd);
        }
    } 
    
    return 0;
}
