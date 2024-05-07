#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

const char template[] = "/tmp/creplXXXXXX";
const int suffixlen = 2;
const char expr_func_nametemp[] = "__expr_wrapper_";
const char expr_func_suffix[] = ";}";
int expr_cnt = 0;

int null_fd = 0;
#define STDOUT_FIFENO 1
#define STDERR_FIFENO 2
// char template[] = "/tmp/creplXXXXXX";
// char filename[50];
// char dyfilename[50];

char* compile(const char* filename){
    int pid = fork();
    static char dyfilename[50];
    sprintf(dyfilename, "%s.so", filename);
    if(pid == 0){
        // dup2(null_fd, STDOUT_FIFENO);
        // dup2(null_fd, STDERR_FIFENO);
        //freopen("/dev/null", "w", stdout);
        //freopen("/dev/null", "w", stderr);
        execlp("gcc", "gcc", "-shared", "-fPIC", "-w", "-o", dyfilename, filename, NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else{
        int status = 0;
        wait(&status);
        if(WIFEXITED(status)){
            printf("gcc: compile error!\n");
            return NULL;
        }
        return dyfilename;
    }
}

void dyload(const char* funcname){

}

int main(int argc, char *argv[]) {
    static char line[4096];
    null_fd = open("/dev/null", O_WRONLY);

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        bool func = false;
        char filename[50];
        char *dyfilename;
        strcpy(filename, template);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        // printf("Got %zu chars.\n", strlen(line));

        if(strncmp(line, "int", 3) == 0){
            func = true;
        }

        int fd = mkstemp(filename);
        if (fd == -1) {
            perror("mkstemp");
            exit(EXIT_FAILURE);
        }

        char expr_func_prev[50] = "";

        if(func){
            write(fd, line, strlen(line));
            
            dyfilename = compile(filename);
        }
        else{
            sprintf(expr_func_prev, "int %s%d() { return ", expr_func_nametemp, expr_cnt);
            write(fd, expr_func_prev, strlen(expr_func_prev));
            write(fd, line, strlen(line));
            write(fd, expr_func_suffix, strlen(expr_func_suffix));
            
            dyfilename = compile(filename);
        }


        

        

        


        // To be implemented.
        
        close(fd);
        //unlink(filename);
        if(dyfilename != NULL){
            unlink(dyfilename);
        }
    }
}
