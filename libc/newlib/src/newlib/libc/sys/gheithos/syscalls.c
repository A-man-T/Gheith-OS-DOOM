#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

static inline long __syscall0(long n) {
    unsigned long __ret;
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
    unsigned long __ret;
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
    unsigned long __ret;
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
    unsigned long __ret;
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
    unsigned long __ret;
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
    unsigned long __ret;
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
    unsigned long __ret;
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

void _exit(int code) {
    __syscall1(0, code);
}

int close(int file) {
    return __syscall1(1007, file);
}

char *environ[] = {0};
int execve(char *name, char **argv, char **env) {
    // TODO
    return -1;
}

int fork() {
    return __syscall0(2);
}

int fstat(int file, struct stat *st) {
    // TODO
    return 0;
}

int getpid() {
    // TODO
    return 1;
}

int isatty(int file) {
    // TODO
    return 1;
}

int kill(int pid, int sig) {
    // TODO pid
    return __syscall1(1027, sig);
}

int link(char *old, char *new) {
    // TODO
    return -1;
}

int lseek(int file, int ptr, int dir) {
    return 0;
}

int open(const char *name, int flags, ...) {
    // TODO flags
    // TODO varargs???
    return __syscall1(1021, (long)name);
}

int read(int file, char *ptr, int len) {
    return __syscall3(1024, file, (long)ptr, len);
}

caddr_t sbrk(int incr) {
    if (incr % 4096 != 0) {
        incr += 4096 - (incr % 4096);
    }

    const char* stack_low = (char*)(0xF0000000-1024*1024);
    extern char _end;
    static char*heap_end = 0;
    char* prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }

    if (heap_end + incr > stack_low) {
        return (caddr_t)-1;
    }

    // mmap the new segment
    __syscall3(1005, (long)heap_end, incr, -1);
    heap_end += incr;

    return (caddr_t)prev_heap_end;
}

int stat(const char *file, struct stat *st) {
    // TODO
    return -1;
}

clock_t times(struct tms *buf) {
    // TODO
    return -1;
}

int unlink(char *name) {
    // TODO
    return -1;
}

int wait(int *status) {
    // TODO
    return -1;
}

int write(int file, char *ptr, int len) {
    return __syscall3(1025, file, (long)ptr, len);
}

int gettimeofday(struct timeval *p, void *z) {
    // TODO
    return 0;
}
