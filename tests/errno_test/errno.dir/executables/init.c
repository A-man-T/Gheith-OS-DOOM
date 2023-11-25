#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/gheithos.h>
#include <unistd.h>

#include "asm.h"

#define SUCCESS 0

#define USER_MEM_START 0x80000000
#define SHARED_ADDRESS 0xF0000000
#define USER_MEM_END SHARED_ADDRESS + 4096
#define USER_STACK_SIZE 1 * 1024 * 1024
#define USER_STACK_START SHARED_ADDRESS
#define USER_STACK_END USER_STACK_START - USER_STACK_SIZE

#define SYS_EXECL 1000
#define SYS_KILL 1027
#define SYS_SEM 1001
#define SYS_UP 1002
#define SYS_DOWN 1003
#define SYS_SEM_CLOSE 1007
#define SYS_SIMPLE_SIGNAL 1004
#define SYS_SIMPLE_MMAP 1005
#define SYS_SIMPLE_MUNMAP 1008
#define SYS_CHDIR 1020
#define SYS_OPEN 1021
#define SYS_CLOSE 1022
#define SYS_LEN 1023
#define SYS_READ 1024
#define SYS_WRITE 1025
#define SYS_PIPE 1026
#define SYS_LSEEK 1029
#define SYS_DUP 1028
#define SYS_IS_PRESSED 2000
#define SYS_IS_HELD 2004

#define ASSERT(c)                                                 \
    do {                                                          \
        if (!(c)) {                                               \
            printf("*** failure at %s:%d\n", __FILE__, __LINE__); \
            shutdown();                                           \
        }                                                         \
    } while (0)

#define HASERROR(operation, error, failure)                                                      \
    do {                                                                                         \
        int result = (int)operation;                                                             \
        int err = errno;                                                                         \
        if (error != SUCCESS) {                                                                  \
            ASSERT(result == failure);                                                           \
        }                                                                                        \
        if (err != error) {                                                                      \
            printf("*** Got error %d instead of %d at %s:%d\n", err, error, __FILE__, __LINE__); \
            shutdown();                                                                          \
        }                                                                                        \
        errno = SUCCESS;                                                                         \
    } while (0)

static int pipe_fds[2];

static int highEsp(int sysnum, int failure) {
    int result = _highEsp(sysnum);
    if (result < 0) {
        errno = -1 * result;
        return failure;
    }
    return result;
}
static int lowEsp(int sysnum, int failure) {
    int result = _lowEsp(sysnum);
    if (result < 0) {
        errno = -1 * result;
        return failure;
    }
    return result;
}
static int unmappedEsp(int sysnum, int failure) {
    int result = _unmappedEsp(sysnum);
    if (result < 0) {
        errno = -1 * result;
        return failure;
    }
    return result;
}

static void efault(int sysnum, int failure) {
    HASERROR(highEsp(sysnum, failure), EFAULT, failure);
    HASERROR(lowEsp(sysnum, failure), EFAULT, failure);
    HASERROR(unmappedEsp(sysnum, failure), EFAULT, failure);
}

int main(int argc, char **argv) {
    // Test 0: exit()
    // TODO exit test

    // Test 1: join()
    // join when no children
    HASERROR(join(), ECHILD, -1);

    /*
    // Test 2: execl()
    efault(SYS_EXECL, -1);
    // segfault pathname
    HASERROR(execl(NULL, 0), EFAULT, -1);
    // segfault arg
    HASERROR(execl("sbin/init", (void *)1, 0), EFAULT, -1);
    // execl file that does not exist
    HASERROR(execl("doesNotExist", "doesNotExist", 0), ENOENT, -1);
    // execl non-file
    HASERROR(execl("sbin", "sbin", 0), EACCES, -1);
    // execl invalid ELF
    HASERROR(execl("executables/init.c", "executables/init.c", 0), ENOEXEC, -1);
    */

    // Test 3: sem()
    efault(SYS_SEM, -1);
    // create too many sems
    for (int i = 0; i < 100; i++) {
        HASERROR(sem(0), SUCCESS, -1);
    }
    HASERROR(sem(1), EMFILE, -1);
    for (int i = 0; i < 100; i++) {
        HASERROR(sem_close(i), SUCCESS, -1);
    }

    // Test 4: up()
    efault(SYS_UP, -1);
    // up non-open sem
    HASERROR(up(1), EBADF, -1);
    // up too large sem
    HASERROR(up(1000), EBADF, -1);

    // Test 5: down()
    efault(SYS_DOWN, -1);
    // down non-open sem
    HASERROR(down(1), EBADF, -1);
    // down too large sem
    HASERROR(down(1000), EBADF, -1);

    // Test 6: sem_close()
    efault(SYS_SEM_CLOSE, -1);
    // close non-open sem
    HASERROR(sem_close(1), EBADF, -1);
    // close too large sem
    HASERROR(sem_close(1000), EBADF, -1);

    // Test 7: simple_signal()
    efault(SYS_SIMPLE_SIGNAL, -1);
    // segfault handler
    HASERROR(simple_signal(NULL), EFAULT, -1);

    // Test 8: sigreturn()
    // need ASM to call interrupt since not directly accessible through libc interface
    ASSERT(_sigreturn() == -1 * ECANCELED);

    // Test 9: simple_mmap()
    efault(SYS_SIMPLE_MMAP, 0);
    // non-aligned start
    HASERROR(simple_mmap((void *)(USER_MEM_START + 1), 4096, -1, 0), EINVAL, 0);
    // non-aligned size
    HASERROR(simple_mmap((void *)USER_MEM_START, 4095, -1, 0), EINVAL, 0);
    // non-aligned offset
    HASERROR(open("executables/init.c", O_RDONLY), SUCCESS, -1);
    HASERROR(simple_mmap((void *)USER_MEM_START, 4096, 3, 1), EINVAL, 0);
    HASERROR(close(3), SUCCESS, -1);
    // too small address
    HASERROR(simple_mmap((void *)4096, 4096, -1, 0), EINVAL, 0);
    // too large address
    HASERROR(simple_mmap((void *)(USER_MEM_END + 4096), 4096, -1, 0), EINVAL, 0);
    // already mapped start address (from ELF)
    HASERROR(simple_mmap((void *)USER_MEM_START, 4096, -1, 0), EINVAL, 0);
    // already mapped end address (from stakck)
    HASERROR(simple_mmap((void *)USER_STACK_END - 4096, 4096 * 2, -1, 0), EINVAL, 0);
    // mapping exists between start and end
    HASERROR(simple_mmap((void *)0xA0000000, 4096, -1, 0), SUCCESS, 0);
    HASERROR(simple_mmap((void *)0xA0000000 - 4096, 4096 * 2, -1, 0), EINVAL, 0);
    HASERROR(simple_munmap((void *)0xA0000000), SUCCESS, -1);
    // no available mappings left in user space for this size
    HASERROR(simple_mmap(0, USER_MEM_END - USER_MEM_START, -1, 0), ENOMEM, 0);
    // size 0
    HASERROR(simple_mmap((void *)0, 0, -1, 0), EINVAL, 0);
    // non-open fd
    HASERROR(simple_mmap((void *)0, 4096, 3, 0), EBADF, 0);
    // too large fd
    HASERROR(simple_mmap((void *)0, 4096, 1000, 0), EBADF, 0);
    // tty
    HASERROR(simple_mmap((void *)0, 4096, 0, 0), EBADF, 0);
    HASERROR(simple_mmap((void *)0, 4096, 1, 0), EBADF, 0);
    // pipe
    HASERROR(pipe(pipe_fds), SUCCESS, -1);
    HASERROR(simple_mmap((void *)0, 4096, pipe_fds[0], 0), EBADF, 0);
    HASERROR(simple_mmap((void *)0, 4096, pipe_fds[1], 0), EBADF, 0);
    HASERROR(close(pipe_fds[0]), SUCCESS, -1);
    HASERROR(close(pipe_fds[1]), SUCCESS, -1);

    // Test 10: simple_munmap()
    efault(SYS_SIMPLE_MUNMAP, -1);
    // unmap address not mapped
    HASERROR(simple_munmap((void *)0xffffffff), EINVAL, -1);

    // Test 11: chdir()
    efault(SYS_CHDIR, -1);
    // segfault pathname
    HASERROR(chdir(NULL), EFAULT, -1);
    HASERROR(chdir("/"), SUCCESS, -1);

    // Test 12: open()
    efault(SYS_OPEN, -1);
    // segfault pathname
    HASERROR(open(NULL, O_RDONLY), EFAULT, -1);
    // open too many files
    for (int i = 3; i < 100; i++) {
        HASERROR(open("executables/init.c", O_RDONLY), SUCCESS, -1);
    }
    HASERROR(open("executables/init.c", O_RDONLY), EMFILE, -1);
    for (int i = 3; i < 100; i++) {
        HASERROR(close(i), SUCCESS, -1);
    }
    // open file that does not exist
    HASERROR(open("doesNotExist", O_RDONLY), ENOENT, -1);

    // Test 13: close()
    efault(SYS_CLOSE, -1);
    // close non-open fd
    HASERROR(close(3), EBADF, -1);
    // close too large fd
    HASERROR(close(1000), EBADF, -1);


    // Test 14: len()
    efault(SYS_LEN, -1);
    // len of non-open fd
    HASERROR(len(3), EBADF, -1);
    // len of too large fd
    HASERROR(len(1000), EBADF, -1);
    // len of tty
    HASERROR(len(0), EBADF, -1);
    HASERROR(len(1), EBADF, -1);
    // len of pipe
    HASERROR(pipe(pipe_fds), SUCCESS, -1);
    HASERROR(len(pipe_fds[0]), EBADF, -1);
    HASERROR(len(pipe_fds[1]), EBADF, -1);
    HASERROR(close(pipe_fds[0]), SUCCESS, -1);
    HASERROR(close(pipe_fds[1]), SUCCESS, -1);

    // Test 15: read()
    efault(SYS_READ, -1);
    // segfault buf
    HASERROR(read(0, NULL, 1), EFAULT, -1);
    char rbuf[100];
    // read non-open fd
    HASERROR(read(3, rbuf, 1), EBADF, -1);
    // read too large fd
    HASERROR(read(1000, rbuf, 1), EBADF, -1);

    // Test 16: write()
    efault(SYS_WRITE, -1);
    // segfault buf
    HASERROR(write(1, NULL, 1), EFAULT, -1);
    char wbuf[100];
    // write non-open fd
    HASERROR(write(3, wbuf, 1), EBADF, -1);
    // write too large fd
    HASERROR(write(1000, wbuf, 1), EBADF, -1);

    // Test 17: pipe()
    efault(SYS_PIPE, -1);
    // segfault fds
    HASERROR(pipe(NULL), EFAULT, -1);
    // open too many files (no room for any extra)
    for (int i = 3; i < 100; i++) {
        HASERROR(open("executables/init.c", O_RDONLY), SUCCESS, -1);
    }
    HASERROR(pipe(pipe_fds), EMFILE, -1);
    for (int i = 3; i < 100; i++) {
        HASERROR(close(i), SUCCESS, -1);
    }
    // open too many files (only room for 1 extra)
    for (int i = 3; i < 99; i++) {
        HASERROR(open("executables/init.c", O_RDONLY), SUCCESS, -1);
    }
    HASERROR(pipe(pipe_fds), EMFILE, -1);
    for (int i = 3; i < 99; i++) {
        HASERROR(close(i), SUCCESS, -1);
    }

    // Test 18: dup()
    efault(SYS_DUP, -1);
    // open too many files
    for (int i = 3; i < 100; i++) {
        HASERROR(open("executables/init.c", O_RDONLY), SUCCESS, -1);
    }
    HASERROR(dup(0), EMFILE, -1);
    for (int i = 3; i < 100; i++) {
        HASERROR(close(i), SUCCESS, -1);
    }
    // dup non-open fd
    HASERROR(dup(3), EBADF, -1);
    // dup too large fd
    HASERROR(dup(1000), EBADF, -1);

    // Test 19: kill()
    efault(SYS_KILL, -1);
    // kill when no child
    HASERROR(kill(0, 1), ECHILD, -1);

    // Test 20: lseek()
    efault(SYS_LSEEK, -1);
    // lseek non-open fd
    HASERROR(lseek(3, 1, SEEK_CUR), EBADF, -1);
    // lseek too large fd
    HASERROR(lseek(1000, 1, SEEK_CUR), EBADF, -1);
    // lseek pipe
    HASERROR(pipe(pipe_fds), SUCCESS, -1);
    HASERROR(lseek(pipe_fds[0], 1, SEEK_CUR), EBADF, -1);
    HASERROR(lseek(pipe_fds[1], 1, SEEK_CUR), EBADF, -1);
    HASERROR(close(pipe_fds[0]), SUCCESS, -1);
    HASERROR(close(pipe_fds[1]), SUCCESS, -1);
    // lseek tty
    HASERROR(lseek(0, 1, SEEK_CUR), EBADF, -1);
    HASERROR(lseek(1, 1, SEEK_CUR), EBADF, -1);
    // lseek invalid whence
    HASERROR(open("executables/init.c", O_RDONLY), SUCCESS, -1);
    HASERROR(lseek(3, 1, 3), EINVAL, -1);
    HASERROR(close(3), SUCCESS, -1);

    // Test 21: is_pressed()
    efault(SYS_IS_PRESSED, -1);
    // invalid key IDs
    HASERROR(is_pressed(0), EINVAL, -1);
    HASERROR(is_pressed(126), EINVAL, -1);

    // read_key_event and read_mouse_event have no errors - they either return an event or 0

    // Test 22: is_held()
    efault(SYS_IS_HELD, -1);
    // invalid key IDs
    HASERROR(is_held(0), EINVAL, -1);
    HASERROR(is_held(126), EINVAL, -1);

    puts("*** PASSED");
    shutdown();
}