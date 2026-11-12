#pragma once
#include <cstddef>

template <typename T>
class PreAllocatedVector {
public:
    void configure(size_t capacity) {
        // TODO
    }

    bool push_back(const T& value) {
        // TODO
        return false;
    }

    T& back() {
        // TODO
        return T{};
    }

private:
    size_t max_capacity = 0;
    size_t size = 0;
    // Data storage placeholder
};
