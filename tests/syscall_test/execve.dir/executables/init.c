// GCC is too smart to let me pass 0x1 as an argument to execve so I use the pragma
#pragma GCC diagnostic ignored "-Wstringop-overread"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/gheithos.h>

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
    chdir("/sbin");

    // Test 1: Cases where exec may fail - path does not exist, {path,argv,envp} unmapped
    ASSERT(execve("doesNotExist", argv, envp) < 0);
    ASSERT(execve((char*) 1, argv, envp) < 0);
    ASSERT(execve("second", (char**) 1, envp) < 0);
    ASSERT(execve("second", argv, (char**) 1) < 0);

    // Test 2: Successful exec
    char *secondArgv[] = {"argv0", "argv1", NULL};
    char *secondEnvp[] = {"envp0", "envp1", NULL};
    execve("second", secondArgv, secondEnvp);
    puts("*** Was not able to exec to second");
}