#pragma once

#include "atomic.h"
#include "events.h"
#include "queue.h"
#include "semaphore.h"

// A bounded buffer is a generalization of a channel
// that has a buffer size "n > 0"
template <typename T>
class BoundedBuffer {
   private:
    const uint32_t size;
    Semaphore senders;
    Semaphore receivers;
    T* buffer;
    Atomic<uint32_t> head{0};
    Atomic<uint32_t> tail{0};
    Atomic<uint32_t> committed_adds{0};
    Atomic<uint32_t> committed_removes{0};

   public:
    // construct a BB with a buffer size of n
    BoundedBuffer(uint32_t n) : size{n}, senders{n}, receivers{0}, buffer{new T[n]} {}
    BoundedBuffer(const BoundedBuffer&) = delete;

    // When room is available in the buffer
    //    - put "v" in the next slot
    //    - schedule a call "work()"
    // Returns immediately
    template <typename Work>
    void put(T v, const Work& work) {
        senders.down([this, v, work] {
            uint32_t previous_index = tail.fetch_add_mod(1, size);
            uint32_t add_index = (previous_index + 1) % size;
            buffer[add_index] = v;

            // This atomic loop (and the similar one in get) is used to commit
            // the reads/writes as finished in order, otherwise, there is a bad
            // race condition with the atomic indices being used if read/writes
            // finish earlier on higher indices than the lowest one in use and
            // open up the opposite semaphore. While a SpinLock could be used
            // instead of this spinning, this spinning allows multiple
            // reads/writes (most importantly, their copy constructors) to run
            // in parallel, so it should end up being more efficient.
            while (true) {
                if (committed_adds.cmp_exchange(previous_index, add_index)) {
                    break;
                }
            }

            receivers.up();
            work();
        });
    }

    // When the buffer is not empty
    //    - remove the first value "v"
    //    - schedule a call to "work(v)"
    // Returns immediately
    template <typename Work>
    void get(const Work& work) {
        receivers.down([this, work] {
            uint32_t previous_index = head.fetch_add_mod(1, size);
            uint32_t remove_index = (previous_index + 1) % size;
            T value = buffer[remove_index];
            buffer[remove_index] = T{};

            while (true) {
                if (committed_removes.cmp_exchange(previous_index, remove_index)) {
                    break;
                }
            }

            senders.up();
            work(value);
        });
    }

    ~BoundedBuffer() {
        delete[] buffer;
    }
};
