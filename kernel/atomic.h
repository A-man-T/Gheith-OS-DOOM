#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include "init.h"
#include "loop.h"
#include "machine.h"

template <typename T>
class AtomicPtr {
    volatile T* ptr;

   public:
    AtomicPtr() : ptr(nullptr) {}
    AtomicPtr(T* x) : ptr(x) {}
    AtomicPtr<T>& operator=(T v) {
        __atomic_store_n(ptr, v, __ATOMIC_SEQ_CST);
        return *this;
    }
    operator T() const {
        return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
    }
    T fetch_add(T inc) {
        return __atomic_fetch_add(ptr, inc, __ATOMIC_SEQ_CST);
    }
    T add_fetch(T inc) {
        return __atomic_add_fetch(ptr, inc, __ATOMIC_SEQ_CST);
    }
    void set(T inc) {
        return __atomic_store_n(ptr, inc, __ATOMIC_SEQ_CST);
    }
    T get(void) {
        return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
    }
    T exchange(T v) {
        T ret;
        __atomic_exchange(ptr, &v, &ret, __ATOMIC_SEQ_CST);
        return ret;
    }
};

template <typename T>
class Atomic {
    volatile T value;

   public:
    Atomic() : value(0) {}
    Atomic(T x) : value(x) {}
    Atomic<T>& operator=(T v) {
        __atomic_store_n(&value, v, __ATOMIC_SEQ_CST);
        return *this;
    }
    operator T() const {
        return __atomic_load_n(&value, __ATOMIC_SEQ_CST);
    }
    T fetch_add(T inc) {
        return __atomic_fetch_add(&value, inc, __ATOMIC_SEQ_CST);
    }
    T add_fetch(T inc) {
        return __atomic_add_fetch(&value, inc, __ATOMIC_SEQ_CST);
    }
    void set(T inc) {
        return __atomic_store_n(&value, inc, __ATOMIC_SEQ_CST);
    }
    T get(void) {
        return __atomic_load_n(&value, __ATOMIC_SEQ_CST);
    }
    T exchange(T v) {
        T ret;
        __atomic_exchange(&value, &v, &ret, __ATOMIC_SEQ_CST);
        return ret;
    }
    bool cmp_exchange(T change_from, T change_to) {
        return __atomic_compare_exchange(&value, &change_from, &change_to, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    }
    T fetch_add_mod(T inc, T mod) {
        while (true) {
            T val = get();
            if (cmp_exchange(val, (val + inc) % mod)) {
                return val;
            }
        }
    }
    T add_mod_fetch(T inc, T mod) {
        while (true) {
            T val = get();
            T ret = (val + inc) % mod;
            if (cmp_exchange(val, ret)) {
                return ret;
            }
        }
    }
    void monitor_value() {
        monitor((uintptr_t)&value);
    }
};

template <>
class Atomic<uint64_t> {
   public:
    Atomic() = delete;
    Atomic(uint64_t) = delete;
};

template <>
class Atomic<int64_t> {
   public:
    Atomic() = delete;
    Atomic(int64_t) = delete;
};

template <typename T>
class LockGuard {
    T& it;

   public:
    inline LockGuard(T& it) : it(it) {
        it.lock();
    }
    inline ~LockGuard() {
        it.unlock();
    }
};

template <typename T>
class LockGuardP {
    T* it;

   public:
    inline LockGuardP(T* it) : it(it) {
        if (it) it->lock();
    }
    inline ~LockGuardP() {
        if (it) it->unlock();
    }
};

class NoLock {
   public:
    inline void lock() {}
    inline void unlock() {}
};

extern void pause();

class SpinLock {
    Atomic<bool> taken;

   public:
    SpinLock() : taken(false) {}

    SpinLock(const SpinLock&) = delete;

    // for debugging, etc. Allows false positives
    bool isMine() {
        return taken.get();
    }

    void lock(void) {
        taken.monitor_value();
        while (taken.exchange(true)) {
            iAmStuckInALoop(true);
            taken.monitor_value();
        }
    }

    void unlock(void) {
        taken.set(false);
    }
};

#endif
