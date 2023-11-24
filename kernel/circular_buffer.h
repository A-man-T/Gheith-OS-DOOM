#ifndef _CIRC_H
#define _CIRC_H

#include "atomic.h"
#include "stdint.h"

template <typename T>
class CircularBuffer {
    private:
        uint32_t size;
        uint32_t head;
        uint32_t tail;
        T* ar;
        SpinLock lock;
    public:
        CircularBuffer(uint32_t size) : size(size){
            LockGuard g(lock);
            ar = new T[size];
            head = 0;
            tail = 0;
        }

    bool add(T el){ //return false if it's full and cant be added
        LockGuard g(lock);
        if ((head + 1) % size == tail){
            return false;
        } 
        ar[head] = el;
        head++;
        head %= size;
        return true;
    }

    T remove(){
        LockGuard g(lock);
        if (head == tail){
            return (T)0;
        }
        uint32_t original_tail = tail;
        tail = (tail + 1) % size;
        return ar[original_tail];
    }
};

#endif 