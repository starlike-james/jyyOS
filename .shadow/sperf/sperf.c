#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *cmd;
char **args;

int main(int argc, char *argv[]) {
    assert(!argv[argc]);

    cmd = argv[1];
    args = argv + 1;

    printf("%s\n", cmd);
    for (int i = 0; i < argc - 1; i++) {
        assert(args[i]);
        printf("args[%d] = %s\n", i, args[i]);
    }
    assert(!args[argc - 1]);
    const char *path = getenv("PATH");
    printf("%s\n", path);
    return 0;
}
