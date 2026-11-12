#pragma once
#include <vector>
#include <cstddef>

template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        // TODO
    }

    bool push(const T& item) {
        // TODO
        return false;
    }

    bool pop(T& item) {
        // TODO
        return false;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
};
