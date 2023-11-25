#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/gheithos.h>

#define ASSERT(condition)                                                                       \
    if (!(condition)) {                                                                         \
        printf("*** Assertion failed: %s, file %s, line %d\n", #condition, __FILE__, __LINE__); \
        shutdown();                                                                                \
    }

int main(int argc, char **argv) {
    int pipe_fds[2];
    pipe(pipe_fds);
    int *pipe_read = &pipe_fds[0];
    int *pipe_write = &pipe_fds[1];

    if (fork() == 0) {
        for (int i = 0; i < 4096; i++) {
            ASSERT(write(*pipe_write, "a", 1) == 1);
        }
        puts("*** Wrote 4096 characters to the pipe");
        // This write should permanently block
        write(*pipe_write, "a", 1);
        puts("*** Wrote 4097 chars");
    } else {
        close(*pipe_read);
        close(*pipe_write);

        pipe(pipe_fds);

        for (int i = 0; i < 4096; i++) {
            ASSERT(write(*pipe_write, "b", 1) == 1);
        }
        puts("*** Wrote 4096 characters to the pipe");
        for (int i = 0; i < 4096; i++) {
            char b;
            ASSERT(read(*pipe_read, &b, 1) == 1);
            ASSERT(b == 'b');
        }
        puts("*** Read 4096 characters from the pipe");

        for (int i = 0; i < 4096; i++) {
            ASSERT(write(*pipe_write, "c", 1) == 1);
        }
        puts("*** Wrote 4096 characters to the pipe");
        for (int i = 0; i < 4096; i++) {
            char c;
            ASSERT(read(*pipe_read, &c, 1) == 1);
            ASSERT(c == 'c');
        }
        puts("*** Read 4096 characters from the pipe");

    }

    shutdown();
}