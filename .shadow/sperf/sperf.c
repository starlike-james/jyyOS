#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

extern char **environ;
const int sec = 1000000;
const int msec = 100000;
// char cmd[100] = "";
// char *path;

#define STDOUT 1
#define STDERR 2
struct timeval last;
struct timeval current;
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
        char **args = malloc(sizeof(char *) * (argc + 2));
        args[0] = "strace";
        args[1] = "-T";
        for(int i = 1; i <= argc; i++){
            args[i + 1] = argv[i];
        }

        const char *PATH = getenv("PATH");
        char *path = malloc(strlen(PATH));
        strcpy(path, PATH);

        char cmd[100] = "";
        char *prefix = strtok(path, ":");
        while (prefix != NULL) {
            sprintf(cmd, "%s/strace", prefix);
            execve(cmd, args, env);
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
        gettimeofday(&last, NULL);
        while (fgets(buf, sizeof(buf), stream) != NULL) {
            gettimeofday(&current, NULL);
            long elapsed = (current.tv_sec - last.tv_sec) * sec + (current.tv_usec - last.tv_usec);
            if(elapsed >= msec){
                printf("Hello, every 0.1 seconds!\n");
                last = current;
            }
         }
        fclose(stream);

    }

    return 0;
}
