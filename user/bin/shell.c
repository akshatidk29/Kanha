#include "stdio.h"
#include "ulib.h"
#include "string.h"

#define MAX_CMD 128
#define MAX_ARGS 16
#define MAX_HISTORY 32

static char history[MAX_HISTORY][MAX_CMD];
static int historyCount = 0;

static void historyAdd(const char *cmd) {
    if (!cmd || cmd[0] == '\0')
        return;

    // Avoid duplicate
    if (historyCount > 0) {
        int lastIdx = (historyCount - 1) % MAX_HISTORY;
        if (strcmp(history[lastIdx], cmd) == 0)
            return;
    }

    int idx = historyCount % MAX_HISTORY;
    strncpy(history[idx], cmd, MAX_CMD - 1);
    history[idx][MAX_CMD - 1] = '\0';
    historyCount++;
}

static int getCmdLine(char *buf, size_t maxLen) {
    if (!buf || maxLen <= 1) 
        return 0;

    int browseIdx = historyCount;
    char currentDraft[MAX_CMD];
    currentDraft[0] = '\0';

    size_t i = 0;
    while (i < maxLen - 1) {
        char c = getchar();
        if (c == '\r' || c == '\n') {
            break;
        } else if (c == 3) { // Ctrl+C
            putchar('^');
            putchar('C');
            putchar('\n');
            buf[0] = '\0';
            return -1;
        } else if (c == 4) { // Ctrl+D
            if (i == 0) {
                return -2;
            }
        } else if (c == 21) { // Ctrl+U
            while (i > 0) {
                putchar('\b');
                putchar(' ');
                putchar('\b');
                i--;
            }
        } else if (c == '\033') {
            char next1 = getchar();
            if (next1 == '[') {
                char next2 = getchar();
                if (next2 == 'A') { // Up Arrow
                    int minBrowse = (historyCount > MAX_HISTORY) ? (historyCount - MAX_HISTORY) : 0;
                    if (browseIdx > minBrowse) {
                        if (browseIdx == historyCount) {
                            buf[i] = '\0';
                            strcpy(currentDraft, buf);
                        }
                        browseIdx--;
                        while (i > 0) {
                            putchar('\b');
                            putchar(' ');
                            putchar('\b');
                            i--;
                        }
                        strcpy(buf, history[browseIdx % MAX_HISTORY]);
                        printf("%s", buf);
                        i = strlen(buf);
                    }
                } else if (next2 == 'B') { // Down Arrow
                    if (browseIdx < historyCount) {
                        browseIdx++;
                        while (i > 0) {
                            putchar('\b');
                            putchar(' ');
                            putchar('\b');
                            i--;
                        }
                        if (browseIdx == historyCount) {
                            strcpy(buf, currentDraft);
                        } else {
                            strcpy(buf, history[browseIdx % MAX_HISTORY]);
                        }
                        printf("%s", buf);
                        i = strlen(buf);
                    }
                }
            }
        } else if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        } else if (c >= 32 && c <= 126) {
            buf[i++] = c;
        }
    }
    buf[i] = '\0';
    return (int)i;
}

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
    char cwd[256];
    
    while (1) {
        getCwd(cwd);
        printf(COLOR_GREEN "Kanha OS:" COLOR_CYAN "%s$ " COLOR_RESET, cwd);

        char input[MAX_CMD];
        int len = getCmdLine(input, sizeof(input));

        if (len < 0) {
            if (len == -2) {
                printf("exit\n");
                break;
            }
            continue; 
        }

        int hasNonSpace = 0;
        for (int k = 0; input[k] != '\0'; k++) {
            if (input[k] != ' ' && input[k] != '\t') {
                hasNonSpace = 1;
                break;
            }
        }
        if (!hasNonSpace) {
            continue;
        }

        // Save command to history
        historyAdd(input);

        char *argv[MAX_ARGS];
        int argc = parseArgs(input, argv);

        if (argc == 0) {
            continue;
        }

        if (strcmp(argv[0], "exit") == 0) {
            printf("Exiting\n");
            break;
        } else if (strcmp(argv[0], "pwd") == 0) {
            char cwd[256];
            getCwd(cwd);
            printf("%s\n", cwd);
            continue;
        } else if (strcmp(argv[0], "cd") == 0) {
            const char *target = (argc > 1) ? argv[1] : "/";
            if (chDir(target) != 0) {
                printf("cd: %s: No such file or directory\n", target);
            }
            continue;
        } else if (strcmp(argv[0], "history") == 0) {
            int start = (historyCount > MAX_HISTORY) ? (historyCount - MAX_HISTORY) : 0;
            for (int h = start; h < historyCount; h++) {
                printf("%d  %s\n", h + 1, history[h % MAX_HISTORY]);
            }
            continue;
        }

        // Execute
        int id = fork();

        if (id == 0) {
            if (argv[0][0] == '/' || (argv[0][0] == '.' && argv[0][1] == '/')) {
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
