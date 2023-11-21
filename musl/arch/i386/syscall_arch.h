#define __SYSCALL_LL_E(x)        \
    ((union { long long ll; long l[2]; }){.ll = x}).l[0], \
        ((union { long long ll; long l[2]; }){.ll = x}).l[1]
#define __SYSCALL_LL_O(x) __SYSCALL_LL_E((x))

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

#define VDSO_USEFUL
#define VDSO_CGT32_SYM "__vdso_clock_gettime"
#define VDSO_CGT32_VER "LINUX_2.6"
#define VDSO_CGT_SYM "__vdso_clock_gettime64"
#define VDSO_CGT_VER "LINUX_2.6"
