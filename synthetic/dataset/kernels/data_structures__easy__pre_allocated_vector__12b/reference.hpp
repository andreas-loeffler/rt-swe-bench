#pragma once
#include <vector>
#include <memory>
#include <stdexcept>

template <typename T>
class PreAllocatedVector {
public:
    void configure(size_t capacity) {
        data.resize(capacity);
        max_capacity = capacity;
        size = 0;
    }

    bool push_back(const T& value) {
        if (size >= max_capacity) {
            return false;
        }
        data[size++] = value;
        return true;
    }

    T& back() {
        return data[size - 1];
    }

private:
    std::vector<T> data;
    size_t max_capacity = 0;
    size_t size = 0;
};
