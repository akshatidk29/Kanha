#include "stdio.h"
#include "ulib.h"
#include "string.h"

#define MAX_CMD 128
#define MAX_ARGS 16

static int parseArgs(char *cmd, char *argv[]) {

    int argc = 0;

    while (*cmd != '\0' && argc < MAX_ARGS - 1) {

        while (*cmd == ' ' || *cmd == '\t') {
            cmd++;
        }
        if (*cmd == '\0') {
            break;
        }

        argv[argc++] = cmd;

        // end of token
        while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t') {
            cmd++;
        }

        // terminate token
        if (*cmd != '\0') {
            *cmd = '\0';
            cmd++;
        }
    }
    argv[argc] = NULL;
    return argc;
}

int main(void) {  

    while (1) {
        printf(COLOR_GREEN "Kanha OS> " COLOR_RESET);

        char input[MAX_CMD];
        int len = getline(input, sizeof(input));
        if (len < 0) {
            if (len == -2) {
                printf("exit\n");
                break;
            }
            continue; 
        }

        char *argv[MAX_ARGS];
        int argc = parseArgs(input, argv);

        if (argc == 0) {
            continue;
        }

        if (strcmp(argv[0], "exit") == 0) {
            printf("Exiting\n");
            break;
        }

        // Execute
        int id = fork();

        if (id == 0) {
            
            if (argv[0][0] == '/') {
                exec(argv[0], argv);
            } else {
                // search in /bin/
                char fullPath[MAX_CMD];
                strcpy(fullPath, "/bin/");
                strcat(fullPath, argv[0]);
                exec(fullPath, argv);
            }

            printf("Command not found: %s\n", argv[0]);
            exit(1);
        } else if (id > 0) {
            int status = 0;
            waitpid(id, &status, 0);
        } else {
            printf("Fork failed\n");
        }
    } 
    
    return 0;
}
