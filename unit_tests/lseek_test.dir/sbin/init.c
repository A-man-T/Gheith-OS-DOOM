#include "libc.h"
#include "stdint.h"

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
    
    char * buffer = (char *) simple_mmap((char *) 0, 4096 * 4, -1, 0); // zerod mmap
    int art_fd = open("/sbin/art");
    
    // test seeking from start
    printf("*** Test 1:\n");
    int new_offset = seek(art_fd, 100, 0); // Use seek to move offset to 100 from the start!
    if (new_offset != 100) {
        printf("*** Error: new_offset wrong! Returned %d\n", new_offset);
        shutdown();
    }
    scan(art_fd, buffer, 100); // Read the first 4096 bytes
    print(2, buffer, 100); // Print first 4096 bytes
    printf("\n");

    // test seeking from offset
    printf("*** Test 2:\n");
    new_offset = seek(art_fd, 200, 1); // Use seek to move offset to 200 from the current offset!
    if (new_offset != 200 + 100 + 100) {
        printf("*** Error: new_offset wrong! Returned %d\n", new_offset);
        shutdown();
    }
    scan(art_fd, buffer, 200); // Read the first 4096 bytes
    print(2, buffer, 200); // Print first 4096 bytes
    printf("\n");



    // test seeking from end
    printf("*** Test 3:\n");
    new_offset = seek(art_fd, -4096, 2); // Use seek to move offset to 4096 from the end!
    if (new_offset != len(art_fd) - 4096) {
        printf("*** Error: new_offset wrong! Returned %d\n", new_offset);
        shutdown();
    }
    scan(art_fd, buffer, 4096); // Read the last 4096 bytes
    print(2, buffer, 4096); // Print last 4096 bytes
    printf("\n");


    // Edge case and debug message testing
    int wrong_fd = seek(9, 10, 0);
    if (wrong_fd != -1) {
        printf("*** Error: shouldn't be able to open invalid fd! Returned %d\n", wrong_fd);
        shutdown();
    }

    wrong_fd = seek(-1, 10, 0);
    if (wrong_fd != -1) {
        printf("*** Error: shouldn't be able to open negative (invalid) fd! Returned %d\n", wrong_fd);
        shutdown();
    }

    wrong_fd = seek(1, 10, 4);
    if (wrong_fd != -1) {
        printf("*** Error: shouldn't be able to open with invalid whence! Returned %d\n", wrong_fd);
        shutdown();
    }


    printf("*** bye!\n");
    shutdown();
}