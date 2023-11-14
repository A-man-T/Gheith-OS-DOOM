#pragma once

template <typename T>
struct Deleter {
    static void free_pointer(T ptr) {
        delete ptr;
    }
};

template <typename T>
struct Deleter<T[]> {
    static void free_pointer(T* ptr) {
        delete[] ptr;
    }
};

template <typename T, typename DeleteType = T*>
class UniquePtr {
   private:
    T* ptr{nullptr};
    bool should_free{true};

   public:
    UniquePtr() {}
    UniquePtr(T* ptr) : ptr{ptr} {}
    UniquePtr(UniquePtr<T>&& other) : ptr{other.ptr}, should_free{other.should_free} {
        other.ptr = nullptr;
    };

    UniquePtr(const UniquePtr<T>& other) = delete;

    UniquePtr<T>& operator=(UniquePtr<T>& other) {
        try_free();

        ptr = other.ptr;
        should_free = other.should_free;
        other.ptr = nullptr;

        return *this;
    }

    UniquePtr<T>& operator=(UniquePtr<T>&& other) {
        try_free();

        ptr = other.ptr;
        should_free = other.should_free;
        other.ptr = nullptr;

        return *this;
    }

    UniquePtr<T>& operator=(T* new_ptr) {
        try_free();

        ptr = new_ptr;
        should_free = true;

        return *this;
    }

    operator T*() {
        return ptr;
    }

    operator const T*() const {
        return ptr;
    }

    T& operator*() {
        return *ptr;
    }

    const T& operator*() const {
        return *ptr;
    }

    T* operator->() {
        return ptr;
    }

    const T* operator->() const {
        return ptr;
    }

    T* release() {
        T* released = ptr;
        ptr = nullptr;
        should_free = false;
        return released;
    }

    ~UniquePtr() {
        try_free();
    }

   private:
    void try_free() {
        if (should_free) {
            Deleter<DeleteType>::free_pointer(ptr);
        }
    }
};