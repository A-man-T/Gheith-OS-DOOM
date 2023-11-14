#pragma once

#include "kernel.h"

namespace Utils {

template <typename T>
T&& move(T& value) {
    return static_cast<T&&>(value);
}

template <typename T>
int32_t validate_and_count(T* arr, bool skip_validation = false) {
    if (!skip_validation && !validate_address(arr)) {
        return -1;
    }

    uint32_t count = 0;
    while (arr[count] != 0) {
        count++;
        if (!skip_validation && !validate_address(arr + count)) {
            return -1;
        }
    }

    return count;
}

}  // namespace Utils