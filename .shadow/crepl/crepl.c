#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

const char template[] = "/tmp/creplXXXXXX.c";
const int suffixlen = 2;
// char template[] = "/tmp/creplXXXXXX";

int main(int argc, char *argv[]) {
    static char line[4096];

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        bool func = false;
        char temp[strlen(template) + 1];
        strcpy(temp, template);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        printf("Got %zu chars. %s\n", strlen(line), line);

        if(strncmp(line, "int", 3) == 0){
            func = true;
        }

        int fd = mkstemps(temp, suffixlen);
        if (fd == -1) {
            perror("mkstemp");
            exit(EXIT_FAILURE);
        }

        write(fd, line, strlen(line));

        // int pid = fork();
        // if(pid == 0){
        //     execlp("gcc", "gcc", "-shared", "-fPIC", "-w", "-o", "libtemp.so", "temp.c", NULL);
        //     perror("execlp");
        //     exit(EXIT_FAILURE);
        // }

        

        

        


        // To be implemented.
        
        close(fd);
        // if(remove(temp) == -1){
        //      perror("remove");
        //      exit(EXIT_FAILURE);
        // }
    }
}
