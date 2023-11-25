#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/gheithos.h>
#include <unistd.h>

// This test case checks basic lseek syscall implementation, as well as some edge cases

// helpers from 003
int print(int fd, char* buffer, int count) {
    int res = 0;
    while (1) {
        int sz = write(fd, (char*)((long)buffer + res), count - res);
        if (sz <= 0) return res;
        res += sz;
        if (res >= count) return count;
    }
    return res;
}

int scan(int fd, char* buffer, int count) {
    int res = 0;
    while (1) {
        int sz = read(fd, (char*)((long)buffer + res), count - res);
        if (sz <= 0) return res;
        res += sz;
        if (res >= count) return count;
    }
    return res;
}

int main(int argc, char** argv) {
    // Test 1:
    printf("*** Test 1: opening up 100 file directories:\n");
    for (int i = 3; i < 100; i++) {
        int a = open("/sbin/art", 0);
        if (a < 0) {
            printf("*** Error: open failed on iteration %d\n", i);
            shutdown();
        }
        if (a != i) {
            printf("*** Error: open returned wrong fd on iteration %d\n", i);
            shutdown();
        }
    }
    printf("*** Passed test 1\n");

    // Test 2:
    printf("*** Test 2: opening up the 101st file directory returns -1:\n");
    int a = open("/sbin/art", 0);
    if (a >= 0) {
        printf("*** Error: open succeeded when it should have failed\n");
        shutdown();
    }
    printf("*** Passed test 2\n");

    // Test 3:
    printf("*** Test 3: closing and getting lengths of file directories > 10:\n");
    a = close(10);
    if (a < 0) {
        printf("*** Error: close failed on fd 10\n");
        shutdown();
    }
    a = len(11);
    if (a != 26276) {
        printf("*** Error: len returned length %d instead of 26726 for fd #11\n", a);
        shutdown();
    }
    a = close(99);
    if (a < 0) {
        printf("*** Error: close failed on fd 99\n");
        shutdown();
    }
    a = len(55);
    if (a != 26276) {
        printf("*** Error: len returned length %d instead of 26726 for fd #55\n", a);
        shutdown();
    }
    printf("*** Passed test 3\n");

    // Test 4:
    printf("*** Test 4: making sure closed fds can be allocated again:\n");
    a = open("/sbin/art", 0);
    if (a != 10) {
        printf("*** Error: open returned wrong fd: got %d when expecting 10\n", a);
        shutdown();
    }
    a = open("/sbin/art", 0);
    if (a != 99) {
        printf("*** Error: open returned wrong fd: got %d when expecting 99\n", a);
        shutdown();
    }
    printf("*** Passed test 4\n");

    // Test 5:
    printf("*** Test 5: making sure all other fds > 10 can be closed properly:\n");
    for (int i = 11; i < 99; i++) {
        a = close(i);
        if (a < 0) {
            printf("*** Error: close failed on fd %d\n", i);
            shutdown();
        }
    }
    printf("*** Passed test 5\n");
    shutdown();
}