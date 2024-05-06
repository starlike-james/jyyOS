#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

char template[] = "/tmp/creplXXXXXX";

int main(int argc, char *argv[]) {
    static char line[4096];

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        bool func = false;

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        if(strncmp(line, "int", 3) == 0){
            func = true;
        }

        int fd = mkstemp(template);

        if (fd == -1) {
            perror("mkstemp");
            exit(EXIT_FAILURE);
        }
        printf("临时文件名：%s\n", template);
        

        

        


        // To be implemented.
        printf("Got %zu chars.\n", strlen(line));
        
        close(fd);

        if(remove(template) == -1){
            perror("remove");
            exit(EXIT_FAILURE);
        }
    }
}
