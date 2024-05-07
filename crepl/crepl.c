#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

const char template[] = "/tmp/creplXXXXXX.c";
const int suffixlen = 2;
const char expr_func_nametemp[] = "__expr_wrapper_";
const char expr_func_suffix[] = ";}";
int expr_cnt = 0;
// char template[] = "/tmp/creplXXXXXX";
void compile(const char* filename){
    // int pid = fork();
    // if(pid == 0){
    //     execlp("gcc", "gcc", "-shared", "-fPIC", "-w", "-o", "libtemp.so", filename, NULL);
    //     perror("execlp");
    //     exit(EXIT_FAILURE);
    // }
}

void dyload(const char* funcname){

}

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

        char expr_func_prev[50] = "";

        if(func){
            write(fd, line, strlen(line));
        }
        else{
            sprintf(expr_func_prev, "int %s%d() { return ", expr_func_nametemp, expr_cnt);
            write(fd, expr_func_prev, strlen(expr_func_prev));
            write(fd, line, strlen(line));
            write(fd, expr_func_suffix, strlen(expr_func_suffix));
        }


        

        

        


        // To be implemented.
        
        close(fd);
        // if(remove(temp) == -1){
        //       perror("remove");
        //       exit(EXIT_FAILURE);
        // }
    }
}
