#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

extern char **environ;
// char cmd[100] = "";
// char *path;
int main(int argc, char *argv[]) {
    char **env = environ;
    pid_t pid = fork();
    if(pid == 0){

        argv[0] = "strace";

        const char *PATH = getenv("PATH");
        char *path = malloc(strlen(PATH));
        strcpy(path, PATH);

        char cmd[100] = "";
        char *prefix = strtok(path, ":");
        while (prefix != NULL){
            sprintf(cmd, "%s/strace", prefix);
            execve(cmd, argv, env);
            prefix = strtok(NULL, ":");
        }
    }


    
    return 0;
}


