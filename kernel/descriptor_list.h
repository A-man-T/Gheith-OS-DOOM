#pragma once

#include "debug.h"
#include "stdint.h"

template <typename T, uint32_t Size>
class DescriptorList {
   private:
    T data[Size];
    uint32_t open_slots{Size};

   public:
    DescriptorList() {}
    DescriptorList(DescriptorList<T, Size>& other) : open_slots{other.open_slots} {
        for (uint32_t i = 0; i < Size; i++) {
            data[i] = other.data[i];
        }
    }

    T* get(uint32_t index) {
        if (index >= Size) {
            return nullptr;
        }

        return &data[index];
    }

    const T* get(uint32_t index) const {
        if (index >= Size) {
            return nullptr;
        }

        return &data[index];
    }

    void set(uint32_t index, T value) {
        if (index >= Size) {
            return;
        }

        if (value != T{} && data[index] == T{}) {
            open_slots--;
        } else if (value == T{} && data[index] != T{}) {
            open_slots++;
        }

        data[index] = value;
    }

    bool remove(uint32_t index) {
        if (get(index) == nullptr) {
            return false;
        }

        set(index, T{});
        return true;
    }

    uint32_t alloc(T value) {
        ASSERT(can_alloc());
        for (uint32_t i = 0; i < Size; i++) {
            if (data[i] == T{}) {
                set(i, value);
                return i;
            }
        }

        MISSING();
        return 0;
    }

    bool can_alloc(uint32_t count = 1) const {
        return open_slots >= count;
    }
};