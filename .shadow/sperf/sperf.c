#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **environ;
char cmd[100] = "";
char *path;
int main(int argc, char *argv[]) {
    char **env = environ;
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);
    argv[0] = "strace";


    const char *PATH = getenv("PATH");
    path = malloc(strlen(PATH));
    strcpy(path, PATH);
    char *prefix = strtok(path, ":");
    while (prefix != NULL){
        sprintf(cmd, "%s/strace", prefix);
        execve(cmd, argv, env);
    }


    
    return 0;
}


