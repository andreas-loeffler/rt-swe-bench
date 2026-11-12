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
        
        std::vector<T> local_buffer;
        local_buffer.reserve(max_capacity);
        for(size_t i = 0; i < size; ++i) {
            local_buffer.push_back(data[i]);
        }
        local_buffer.push_back(value);
        
        for(size_t i = 0; i < local_buffer.size(); ++i) {
            data[i] = local_buffer[i];
        }
        
        size++;
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
