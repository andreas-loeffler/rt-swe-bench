#pragma once
#include <cstddef>
#include <memory>
#include <new>

/**
 * PreAllocatedVector: A real-time safe container.
 * Constraints:
 * - configure(): Non-RT, can allocate.
 * - push_back(), back(): Hard RT, zero allocation, no exceptions, no locks.
 */
template <typename T>
class PreAllocatedVector {
public:
    void configure(size_t capacity) {
        // Clean up previous allocation if configure is called multiple times
        if (data_) {
            for (size_t i = 0; i < size; ++i) {
                data_[i].~T();
            }
            ::operator delete(data_, std::align_val_t{alignof(T)});
        }

        max_capacity = capacity;
        size = 0;

        if (capacity > 0) {
            // Allocate raw memory to avoid calling constructors prematurely
            void* raw = ::operator new(capacity * sizeof(T), std::align_val_t{alignof(T)});
            data_ = static_cast<T*>(raw);
        } else {
            data_ = nullptr;
        }
    }

    bool push_back(const T& value) {
        if (size >= max_capacity) {
            return false;
        }

        // Use placement new to construct the element in the pre-allocated buffer
        // This is O(1) and does not allocate heap memory.
        new (&data_[size]) T(value);
        size++;
        return true;
    }

    T& back() {
        // Precondition: container must not be empty.
        // Accessing the last initialized element.
        return data_[size - 1];
    }

    // Destructor handles the cleanup of the raw buffer
    ~PreAllocatedVector() {
        if (data_) {
            for (size_t i = 0; i < size; ++i) {
                data_[i].~T();
            }
            ::operator delete(data_, std::align_val_t{alignof(T)});
        }
    }

    // Prevent copying to avoid accidental RT violations or double-frees
    PreAllocatedVector(const PreAllocatedVector&) = delete;
    PreAllocatedVector& operator=(const PreAllocatedVector&) = delete;
    PreAllocatedVector() = default;

private:
    size_t max_capacity = 0;
    size_t size = 0;
    T* data_ = nullptr;
};
