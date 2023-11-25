#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/gheithos.h>

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
    
    printf("*** Test 1: testing fds 0, 1, and 2 return isatty = true\n");
    if (isatty(0) != 1) {
        printf("*** Error: isatty(0) should return true\n");
        shutdown();
    }
    if (isatty(1) != 1) {
        printf("*** Error: isatty(1) should return true\n");
        shutdown();
    }
    if (isatty(2) != 1) {
        printf("*** Error: isatty(2) should return true\n");
        shutdown();
    }
    printf("*** Passed test 1\n");
    printf("*** Test 2: testing closing fd 0 or 2 and opening new fds there returns isatty = false\n");
    close(0);
    close(2);
    open("/sbin/art", O_RDONLY); // Should be at 0
    open("/sbin/art", O_RDONLY); // Should be at 2
    if (isatty(0) != 0) {
        printf("*** Error: isatty(0) should return false\n");
        shutdown();
    }
    if (isatty(2) != 0) {
        printf("*** Error: isatty(2) should return false\n");
        shutdown();
    }
    printf("*** Passed test 2\n");
    shutdown();
}