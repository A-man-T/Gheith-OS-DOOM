#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

#include "sys/gheithos.h"

#define SYS_EXIT 0
#define SYS_FORK 2
#define SYS_SHUTDOWN 7
#define SYS_YIELD 998
#define SYS_JOIN 999
#define SYS_EXECVE 1000
#define SYS_KILL 1027
#define SYS_SEM 1001
#define SYS_UP 1002
#define SYS_DOWN 1003
#define SYS_SEM_CLOSE 1007
#define SYS_SIMPLE_SIGNAL 1004
#define SYS_SIGRETURN 1006
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
#define SYS_READ_KEY_EVENT 2001
#define SYS_READ_MOUSE_EVENT 2003
#define SYS_IS_HELD 2004

static inline int setErrno(int __ret) {
    if (__ret < 0) {
        errno = -1 * __ret;
        return -1;
    }
    return __ret;
}

static inline long __syscall0(long n) {
    long __ret;
    __asm__ __volatile__(
        "sub $4, %%esp;"
        "int $48;"
        "add $4,%%esp;"
        : "=a"(__ret)
        : "a"(n)
        : "memory");
    return __ret;
}

static inline long __syscall1(long n, long a1) {
    long __ret;
    __asm__ __volatile__(
        "pushl %2;"
        "sub $4, %%esp;"
        "int $48;"
        "add $8,%%esp;"
        : "=a"(__ret)
        : "a"(n), "r"(a1)
        : "memory");
    return __ret;
}

static inline long __syscall2(long n, long a1, long a2) {
    long __ret;
    __asm__ __volatile__(
        "pushl %3;"
        "pushl %2;"
        "sub $4, %%esp;"
        "int $48;"
        "add $12,%%esp;"
        : "=a"(__ret)
        : "a"(n), "r"(a1), "r"(a2)
        : "memory");
    return __ret;
}

static inline long __syscall3(long n, long a1, long a2, long a3) {
    long __ret;
    __asm__ __volatile__(
        "pushl %4;"
        "pushl %3;"
        "pushl %2;"
        "sub $4, %%esp;"
        "int $48;"
        "add $16,%%esp;"
        : "=a"(__ret)
        : "a"(n), "r"(a1), "r"(a2), "r"(a3)
        : "memory");
    return __ret;
}

static inline long __syscall4(long n, long a1, long a2, long a3, long a4) {
    long __ret;
    __asm__ __volatile__(
        "pushl %5;"
        "pushl %4;"
        "pushl %3;"
        "pushl %2;"
        "sub $4, %%esp;"
        "int $48;"
        "add $20,%%esp;"
        : "=a"(__ret)
        : "a"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4)
        : "memory");
    return __ret;
}

static inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5) {
    long __ret;
    __asm__ __volatile__(
        "pushl %6;"
        "pushl %5;"
        "pushl %4;"
        "pushl %3;"
        "pushl %2;"
        "sub $4, %%esp;"
        "int $48;"
        "add $24,%%esp;"
        : "=a"(__ret)
        : "a"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
        : "memory");
    return __ret;
}

static inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    long __ret;
    __asm__ __volatile__(
        "pushl %7;"
        "pushl %6;"
        "pushl %5;"
        "pushl %4;"
        "pushl %3;"
        "pushl %2;"
        "sub $4, %%esp;"
        "int $48;"
        "add $28,%%esp;"
        : "=a"(__ret)
        : "a"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
        : "memory");
    return __ret;
}

static inline long __syscall_varargs(long n, const void **args) {
    long __ret;
    __asm__ __volatile__(
        "mov %%esp, %%esi;"
        "mov %2, %%esp;"
        "int $48;"
        "mov %%esi, %%esp;"
        : "=a"(__ret)
        : "a"(n), "r"((long)&args[-1])
        : "memory");
    return __ret;
}

// syscalls required by newlib
int _exit(int code) {
    return setErrno(__syscall1(SYS_EXIT, code));
}

int close(int file) {
    return setErrno(__syscall1(SYS_CLOSE, file));
}

char *environ[] = {0};
int _execve(const char *name, const char **argv, const char **env) {
    // TODO environment?

    return setErrno(__syscall3(SYS_EXECVE, name, argv, env));
}

int fork() {
    return setErrno(__syscall0(SYS_FORK));
}

int fstat(int file, struct stat *st) {
    // TODO
    errno = ENOSYS;
    return 0;
}

int getpid() {
    // TODO
    errno = ENOSYS;
    return 1;
}

int isatty(int file) {
    // TODO
    errno = ENOSYS;
    return 1;
}

int kill(int pid, int sig) {
    // TODO kill by pid
    return setErrno(__syscall1(SYS_KILL, sig));
}

int link(char *old, char *new) {
    // TODO
    errno = ENOSYS;
    return -1;
}

int lseek(int file, int ptr, int dir) {
    return setErrno(__syscall3(SYS_LSEEK, file, ptr, dir));
}

int open(const char *name, int flags, ...) {
    // TODO flags
    // TODO varargs?
    return setErrno(__syscall1(SYS_OPEN, (long)name));
}

int read(int file, char *ptr, int len) {
    return setErrno(__syscall3(SYS_READ, file, (long)ptr, len));
}

caddr_t sbrk(int incr) {
    // TODO errno
    if (incr % 4096 != 0) {
        incr += 4096 - (incr % 4096);
    }

    const char *stack_low = (char *)(0xF0000000 - 1024 * 1024);
    extern char _end;
    static char *heap_end = 0;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }

    if (heap_end + incr > stack_low) {
        return (caddr_t)-1;
    }

    // mmap the new segment
    if (simple_mmap(heap_end, incr, -1, 0) == 0) {
        return (caddr_t)-1;
    }
    heap_end += incr;

    return (caddr_t)prev_heap_end;
}

int stat(const char *file, struct stat *st) {
    // TODO
    errno = ENOSYS;
    return -1;
}

clock_t times(struct tms *buf) {
    // TODO
    errno = ENOSYS;
    return -1;
}

int unlink(char *name) {
    // TODO
    errno = ENOSYS;
    return -1;
}

int wait(int *status) {
    // TODO proper wait
    return setErrno(__syscall0(SYS_JOIN));
}

int write(int file, char *ptr, int len) {
    return setErrno(__syscall3(SYS_WRITE, file, (long)ptr, len));
}

int gettimeofday(struct timeval *p, void *z) {
    // TODO
    errno = ENOSYS;
    return 0;
}

// other syscalls
void shutdown(void) {
    setErrno(__syscall0(SYS_SHUTDOWN));
}

void yield(void) {
    setErrno(__syscall0(SYS_YIELD));
}

int join(void) {
    return setErrno(__syscall0(SYS_JOIN));
}

// semaphores
int sem(unsigned int initial_value) {
    return setErrno(__syscall1(SYS_SEM, initial_value));
}

int up(unsigned int num) {
    return setErrno(__syscall1(SYS_UP, num));
}

int down(unsigned int num) {
    return setErrno(__syscall1(SYS_DOWN, num));
}

int sem_close(unsigned int num) {
    return setErrno(__syscall1(SYS_SEM_CLOSE, num));
}

// signals
int simple_signal(void (*handler)(int, unsigned int)) {
    return setErrno(__syscall1(SYS_SIMPLE_SIGNAL, (long)handler));
}

int sigreturn() {
    return setErrno(__syscall0(SYS_SIGRETURN));
}

// mmap
void *simple_mmap(void *address, unsigned int size, int fd, unsigned int offset) {
    long __ret = __syscall4(SYS_SIMPLE_MMAP, (long)address, size, fd, offset);
    if (__ret % 4096 != 0) {
        // https://stackoverflow.com/a/47566663 to check if __ret is error or high address
        errno = -1 * __ret;
        return NULL;
    }
    return (void *)__ret;
}

int simple_munmap(void *address) {
    return setErrno(__syscall1(SYS_SIMPLE_MUNMAP, (long)address));
}

// files
int chdir(const char *path) {
    return setErrno(__syscall1(SYS_CHDIR, (long)path));
}

int len(int fd) {
    return setErrno(__syscall1(SYS_LEN, fd));
}

int pipe(int fds[2]) {
    return setErrno(__syscall2(SYS_PIPE, (long)&fds[1], (long)&fds[0]));
}

int dup(int fd) {
    return setErrno(__syscall1(SYS_DUP, fd));
}


int read_mouse_event(void) {
    return setErrno(__syscall0(SYS_READ_MOUSE_EVENT));
}

int read_key_event(void) {
    return setErrno(__syscall0(SYS_READ_KEY_EVENT));
}

int is_pressed(unsigned int key) {
    return setErrno(__syscall1(SYS_IS_PRESSED, key));
}

int is_held(unsigned int key) {
    return setErrno(__syscall1(SYS_IS_HELD, key));
}