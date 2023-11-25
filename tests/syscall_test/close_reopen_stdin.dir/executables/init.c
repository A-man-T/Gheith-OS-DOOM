#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/gheithos.h>
/*
Very trivial test that makes sure that you can close stdin and open fd0 with something else
*/

#define ASSERT(condition) \
    if (!(condition)) { \
        printf("Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        exit(1); \
    }

int main(int argc, char** argv) {
    char str[256] = {};
    close(0);
    int fd = open("/files/input.txt", O_RDONLY);
    ASSERT(fd == 0);

    printf("*** %s\n", gets(str));
    shutdown();
}