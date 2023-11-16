#include "libc.h"
/*
Very trivial test that makes sure that you can read a single command from stdin 
and echo the commmand back to the user
*/

#define ASSERT(condition) \
    if (!(condition)) { \
        printf("Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        exit(1); \
    }

int main(int argc, char** argv) {
    char str[256] = {};
    close(0);
    int fd = open("/files/input.txt");
    ASSERT(fd == 0);

    printf("*** %s\n", gets(str));
    shutdown();
}