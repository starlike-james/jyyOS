#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <regex.h>
#include <inttypes.h>

extern char **environ;
const int sec = 1000000;
const int msec = 100000;
// char cmd[100] = "";
// char *path;

#define STDOUT 1
#define STDERR 2
struct timeval last;
struct timeval current;

char regex_syscall[20] = "^[^(]*";
char regex_time[20] = "<.*>$";

char* match_regax(const char *line, const char *regex_text){
    regex_t regex;
    int result;
    regmatch_t matches[1];
    // static char matchbuf[100];

    result = regcomp(&regex, regex_text, 0);
    if(result != 0){
        char err[100];
        regerror(result, &regex, err, sizeof(err));
        fprintf(stderr, "Regex error compling '%s': %s\n", regex_text, err);
        return NULL;
    }

    result = regexec(&regex, line, 1, matches, 0);
    regfree(&regex);

    if(result == 0){
        int len = matches[0].rm_eo - matches[0].rm_so;
        char* matchbuf = malloc(len);
        strncpy(matchbuf, line + matches[0].rm_so, matches[0].rm_eo - matches[0].rm_so);
        printf("Matched text: %.*s\n", matches[0].rm_eo - matches[0].rm_so, line + matches[0].rm_so);
        matchbuf[len] = '\0';
        // printf("%zu %d\n", strlen(matchbuf), len);
        assert(strlen(matchbuf) == len);
        return matchbuf;
    }
    else if(result == REG_NOMATCH){
        fprintf(stderr, "No match found.\n");
        return NULL;
    }
    else{
        char err[100];
        regerror(result, &regex, err, sizeof(err));
        fprintf(stderr, "Regex error compling '%s': %s\n", regex_text, err);
        return NULL;
    }
}

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
        for (int i = 1; i <= argc; i++) {
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
            printf("%s\n", buf);
            gettimeofday(&current, NULL);
            long elapsed = (current.tv_sec - last.tv_sec) * sec +
                           (current.tv_usec - last.tv_usec);
            char *syscall = match_regax(buf, regex_syscall);
            char *systime = match_regax(buf, regex_time);
            if(syscall != NULL){
                printf("%s\n", syscall);
                free(syscall);
            }
            if(systime != NULL){
                printf("%s\n", systime);
                free(systime);
            }

            if (elapsed >= msec) {
                last = current;
                fflush(stdout);
            }
        }
        fclose(stream);
    }

    return 0;
}
