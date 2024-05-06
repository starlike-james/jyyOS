#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

const char template[] = "/tmp/creplXXXXXX.c";
const int suffixlen = 2;
const char expr_func_template[] = "__expr_wrapper_";
int expr_cnt = 0;
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

        char expr_func[100] = "";

        if(func){
            write(fd, line, strlen(line));
        }
        else{
            sprintf(expr_func, "int %s%d() { return %s ;}", expr_func_template, expr_cnt, line);
            write(fd, expr_func, strlen(expr_func));
        }

        // int pid = fork();
        // if(pid == 0){
        //     execlp("gcc", "gcc", "-shared", "-fPIC", "-w", "-o", "libtemp.so", "temp.c", NULL);
        //     perror("execlp");
        //     exit(EXIT_FAILURE);
        // }

        

        

        


        // To be implemented.
        
        close(fd);
        if(remove(temp) == -1){
              perror("remove");
              exit(EXIT_FAILURE);
        }
    }
}
