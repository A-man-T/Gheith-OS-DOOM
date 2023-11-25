#include <stddef.h>

#include "libc.h"

#define ASSERT(condition)                                                                       \
    if (!(condition)) {                                                                         \
        printf("*** Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        exit(1);                                                                                \
    }

int main(int argc, char **argv, char **envp) {
    puts("*** Was able to execve with null argv and envp");
    ASSERT(argc == 0);
    ASSERT(argv[0] == NULL);
    ASSERT(envp[0] == NULL);
    exit(0);
}