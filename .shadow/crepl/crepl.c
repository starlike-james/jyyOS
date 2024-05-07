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
// char template[] = "/tmp/creplXXXXXX";
void compile(const char* filename){
    int pid = fork();
    if(pid == 0){
        char dyfilename[50];
        sprintf(dyfilename, "%s.so", filename);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        execlp("gcc", "gcc", "-shared", "-fPIC", "-w", "-o", dyfilename, filename, NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else{
        wait(NULL);
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
        char temp[strlen(template) + 1];
        strcpy(temp, template);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        printf("Got %zu chars. %s\n", strlen(line), line);

        if(strncmp(line, "int", 3) == 0){
            func = true;
        }

        int fd = mkstemp(temp);
        if (fd == -1) {
            perror("mkstemp");
            exit(EXIT_FAILURE);
        }

        char expr_func_prev[50] = "";

        if(func){
            write(fd, line, strlen(line));
            
            compile(temp);
        }
        else{
            sprintf(expr_func_prev, "int %s%d() { return ", expr_func_nametemp, expr_cnt);
            write(fd, expr_func_prev, strlen(expr_func_prev));
            write(fd, line, strlen(line));
            write(fd, expr_func_suffix, strlen(expr_func_suffix));
            
            compile(temp);
        }


        

        

        


        // To be implemented.
        
        close(fd);
        // if(remove(temp) == -1){
        //       perror("remove");
        //       exit(EXIT_FAILURE);
        // }
    }
}
