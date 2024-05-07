#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
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

int compile(const char* filename, const char* dyfilename){
    int pid = fork();
    if(pid == 0){
        dup2(null_fd, STDOUT_FIFENO);
        dup2(null_fd, STDERR_FIFENO);
        //freopen("/dev/null", "w", stdout);
        //freopen("/dev/null", "w", stderr);
        #ifdef __x86_64__
            execlp("gcc", "gcc", "-shared", "-fPIC", "-m64", "-w", "-o", dyfilename, "-x", "c", filename, NULL);
        #else
            execlp("gcc", "gcc", "-shared", "-fPIC", "-m32", "-w", "-o", dyfilename, "-x", "c", filename, NULL);
        #endif

        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else{
        int status = 0;
        wait(&status);
        int exit_status = WEXITSTATUS(status);
        //printf("%d\n", exit_status);
        return exit_status;
    }
}

void* dyload(const char* dyfilename){
    void *handle;
    handle = dlopen(dyfilename, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return NULL;
    }
    dlerror();
    return handle;
}

int main(int argc, char *argv[]) {
    static char line[4096];
    null_fd = open("/dev/null", O_WRONLY);

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        bool is_func = false;
        
        char filename[25] = "";
        char dyfilename[30] = "";

        strcpy(filename, template);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        // printf("Got %zu chars.\n", strlen(line));

        if(strncmp(line, "int", 3) == 0){
            is_func = true;
        }

        int fd = mkstemp(filename);
        if (fd == -1) {
            perror("mkstemp");
            exit(EXIT_FAILURE);
        }

        sprintf(dyfilename, "%s.so", filename);
        
        char expr_func_name[30] = "";
        if(is_func){
            write(fd, line, strlen(line));     
        }
        else{
            char expr_func_prev[50] = "";
            sprintf(expr_func_name, "%s%d", expr_func_nametemp, expr_cnt);
            expr_cnt++;
            sprintf(expr_func_prev, "int %s() { return ", expr_func_name);
            write(fd, expr_func_prev, strlen(expr_func_prev));
            write(fd, line, strlen(line));
            write(fd, expr_func_suffix, strlen(expr_func_suffix));    
        }

        close(fd);

        int exit_status = compile(filename, dyfilename);

        if(exit_status != 0){
            printf("Compile Error.\n");
            unlink(filename);
            continue;
        }
        
        void *handle = dyload(dyfilename);
        
        if(!is_func){
            int (*expr_func)(void);
            char* error;
            if((error = dlerror()) != NULL){
                fprintf(stderr, "%s\n", error);
            }
            else{
                *(void **)(&expr_func) = dlsym(handle, expr_func_name);
            }

            int res = expr_func();

            

            printf("= %d\n", res);
        }
        else{
            printf("OK.\n");
        }

        
        // To be implemented.
        
        unlink(filename);
        unlink(dyfilename);
    }
}
