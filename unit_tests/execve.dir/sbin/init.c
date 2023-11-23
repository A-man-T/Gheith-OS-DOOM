#include <stddef.h>

#include "libc.h"

/**
 * Tests that execve behaves as expected
 */

#define ASSERT(condition)                                                                       \
    if (!(condition)) {                                                                         \
        printf("*** Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        exit(1);                                                                                \
    }

int main(int argc, char **argv, char **envp) {
    puts("*** Was able to exec init!");

    // Test 1: Cases where exec may fail - path does not exist, {path,argv,envp} unmapped
    ASSERT(execve("doesNotExist", argv, envp) < 0);
    ASSERT(execve(NULL, argv, envp) < 0);
    ASSERT(execve("second", NULL, envp) < 0);
    ASSERT(execve("second", argv, NULL) < 0);

    // Test 2: Successful exec
    char *secondArgv[] = {"argv0", "argv1", NULL};
    char *secondEnvp[] = {"envp0", "envp1", NULL};
    execve("/sbin/second", secondArgv, secondEnvp);
    puts("*** Was not able to exec to second");
}