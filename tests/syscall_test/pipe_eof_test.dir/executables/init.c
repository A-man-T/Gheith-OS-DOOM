#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/gheithos.h>

// This test case checks the implementation of pipe, and whether closing the write end, and trying to read returns 0.

int main(int argc, char** argv) {
    /*
    TEST 1
    Ensure if the parent creates a pipe, closes the write end, and child forks and tries to read from the pipe,
    we return a 0 for EOF. 
    */

    int pipe_fds[2];
    pipe(pipe_fds);
    int pipe_write = pipe_fds[0];
    int pipe_read = pipe_fds[1];
    close(pipe_write);

    printf("*** Starting Test 1\n");

    char* buffer = (char *)malloc(10);

    if (fork() == 0) {
        int pipeChildRead = read(pipe_read, buffer, 10);

        if (pipeChildRead == 0) {
            printf("*** Passed Test 1.1\n");
        } else {
            printf("*** Failed Test 1.1\n");
        }

        exit(10);
    } else {
        join();
    }
    //Sanity check.. Still can't read from the parent
    int pipeParentRead = read(pipe_read, buffer, 10);

    if (pipeParentRead == 0) {
        printf("*** Passed Test 1.2\n");
    } else {
        printf("*** Failed Test 1.2\n");
    }

    printf("*** Finished Test 1\n");

    close(pipe_read);
    
    /*
    TEST 2
    If the parent creates a pipe, child inherits it, and child writes to it and closes the fd,
    the parent can still read from it.
    */
    
    printf("*** Starting Test 2\n");

    pipe(pipe_fds);

    if (fork() == 0) {
        write(pipe_write, "h", 1);
        write(pipe_write, "e", 1);

        close(pipe_write);

        exit(10);
    } else {
        join();
    }

    pipeParentRead = read(pipe_read, buffer, 1);

    if (pipeParentRead == 1 && buffer[0] == 'h') {
        printf("*** Passed Test 2\n");
    } else {
        printf("*** Failed Test 2\n");
    }

    printf("*** Finished Test 2\n");

    /*
    TEST 3
    Okay. Let's close the write end of the parent as well.
    We can still read another byte before reaching EOF (desired behavior).
    */
    close(pipe_write);

    printf("*** Starting Test 3\n");

    pipeParentRead = read(pipe_read, buffer, 1);

    if (pipeParentRead == 1 && buffer[0] == 'e') {
        printf("*** Passed Test 3.1\n");
    } else if (pipeParentRead == 0) {
        printf("*** Failed Test 3.1. Pipe reached EOF already.\n");
    } else {
        printf("*** Failed Test 3.1. Unexpected behavior\n");
    }

    //Now, I am officially at the EOF..

    pipeParentRead = read(pipe_read, buffer, 1);

    if (pipeParentRead == 0) {
        printf("*** Passed Test 3.2\n");
    } else {
        printf("*** Failed Test 3.2\n");
    }

    printf("*** Finished Test 3\n");

    close(pipe_read);

    /*
    TEST 4
    If read blocks, and all the writers are closed, the read must get EOF.
    */
    printf("*** Starting Test 4\n");

    pipe(pipe_fds);

    if (fork() == 0) {
        close(pipe_write);
        exit(10);
    } else {
        close(pipe_write);
        //The read is still possible... Blocks the parent
        //The intended behavior is set to be ran with 1 core
        //Multiple cores, while may not break the TC, does not test the intended behavior
        pipeParentRead = read(pipe_read, buffer, 1);

        if (pipeParentRead == 0) {
            printf("*** Passed Test 4\n");
        } else {
            printf("*** Failed Test 4\n");
        }
    }

    printf("*** Finished Test 4\n");

    shutdown();
}