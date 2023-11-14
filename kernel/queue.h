#ifndef _queue_h_
#define _queue_h_

#include "atomic.h"

template <typename SubclassType, typename LockType, typename T = SubclassType>
class Queue {
    T* volatile first = nullptr;
    T* volatile last = nullptr;
    LockType lock;

   public:
    Queue() : first(nullptr), last(nullptr), lock() {}
    Queue(const Queue&) = delete;
    Queue& operator=(Queue&) = delete;

    void monitor_add() {
        monitor((uintptr_t)&last);
    }

    void monitor_remove() {
        monitor((uintptr_t)&first);
    }

    void add(T* t) {
        LockGuard g{lock};
        t->next = nullptr;
        if (first == nullptr) {
            first = t;
        } else {
            last->next = t;
        }
        last = t;
    }

    void add_stack(T* t) {
        LockGuard g{lock};
        t->next = first;
        first = t;
        if (last == nullptr) {
            last = t;
        }
    }

    SubclassType* remove() {
        LockGuard g{lock};
        if (first == nullptr) {
            return nullptr;
        }
        auto it = first;
        first = it->next;
        if (first == nullptr) {
            last = nullptr;
        }
        return static_cast<SubclassType*>(it);
    }

    T* remove_all() {
        LockGuard g{lock};
        auto it = first;
        first = nullptr;
        last = nullptr;
        return it;
    }

    template <typename OtherT, typename OtherLockType, typename OtherNextType>
    friend class Queue;

    template <typename OtherT, typename OtherLockType, typename OtherNextType>
    void transfer_all(Queue<OtherT, OtherLockType, OtherNextType>& other) {
        LockGuard g_other{other.lock};
        if (other.first == nullptr) {
            return;
        }

        LockGuard g{lock};

        if (first == nullptr) {
            first = other.first;
        } else {
            last->next = other.first;
        }
        last = other.last;
        other.first = nullptr;
        other.last = nullptr;
    }

    // get_front and get_next work like iterator methods
    SubclassType* get_front() {
        LockGuard g{lock};
        return static_cast<SubclassType*>(first);
    }

    SubclassType* get_next(SubclassType* item) {
        LockGuard g{lock};
        return static_cast<SubclassType*>(static_cast<T*>(item)->next);
    }

    bool is_empty() {
        return first == nullptr;
    }

    void free_all_items() {
        LockGuard g{lock};
        T* it = first;
        while (it != nullptr) {
            T* next = it->next;
            delete it;
            it = next;
        }
        first = nullptr;
        last = nullptr;
    }
};

#endif
