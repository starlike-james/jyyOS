#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

char *CMD;
int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);

    const char *path = getenv("PATH");
    printf("%s\n", path);
    return 0;
}


