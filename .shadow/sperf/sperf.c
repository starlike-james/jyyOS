#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char **environ;
// char cmd[100] = "";
// char *path;
#define STDERR 2
#define STDOUT 1
int main(int argc, char *argv[]) {
    char **env = environ;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        close(STDERR);
        dup(pipefd[1]);
        close(pipefd[0]);
        close(pipefd[1]);

        close(STDOUT);
        int fd = open("/dev/null", O_WRONLY);
        dup(fd);

        argv[0] = "strace";

        const char *PATH = getenv("PATH");
        char *path = malloc(strlen(PATH));
        strcpy(path, PATH);

        char cmd[100] = "";
        char *prefix = strtok(path, ":");
        while (prefix != NULL) {
            sprintf(cmd, "%s/strace", prefix);
            execve(cmd, argv, env);
            prefix = strtok(NULL, ":");
        }
        printf("strace not found in PATH.\n");
        exit(EXIT_FAILURE);
    } else {
        // close(stdin);
        // dup(pipefd[0]);
        // close(pipefd[0]);
        close(pipefd[1]);

        char buf[1024];
        FILE *stream = fdopen(pipefd[0], "r");
        if (!stream) {
            perror("fdopen");
            exit(EXIT_FAILURE);
        }
        // fgets(buf, sizeof(buf), stream);
        while (fgets(buf, sizeof(buf), stream) != NULL) {
            printf("%s", buf);
        }
        fclose(stream);

    }

    return 0;
}
