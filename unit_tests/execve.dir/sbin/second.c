#include <stddef.h>

#include "libc.h"

#define ASSERT(condition)                                                                       \
    if (!(condition)) {                                                                         \
        printf("*** Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        exit(1);                                                                                \
    }

int main(int argc, char **argv, char **envp) {
    puts("*** Made it to second!");
    ASSERT(argc == 2);
    printf("*** argv[0] = %s\n", argv[0]);
    printf("*** argv[1] = %s\n", argv[1]);
    ASSERT(argv[2] == NULL);
    printf("*** envp[0] = %s\n", envp[0]);
    printf("*** envp[1] = %s\n", envp[1]);
    ASSERT(envp[2] == NULL);
    shutdown();
}