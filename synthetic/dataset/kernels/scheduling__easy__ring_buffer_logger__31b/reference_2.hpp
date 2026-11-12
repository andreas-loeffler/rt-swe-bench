#pragma once
#include <cstddef>
#include <vector>

class RingBufferLogger {
public:
    void configure(size_t capacity) {
        buffer.resize(capacity);
        max_capacity = capacity;
        head = 0;
        count = 0;
    }

    void push(float sample) {
        buffer[head] = sample;
        head = (head + 1) % max_capacity;
        if (count < max_capacity) {
            count++;
        }
    }

    size_t size() const {
        return count;
    }

    float get(size_t index) const {
        // The oldest sample is at 'head' (if full) or '0' (if not full)
        // However, to simplify logic: if count < max_capacity, oldest is 0.
        // If count == max_capacity, oldest is 'head' (the index where we next write).
        
        size_t start_idx;
        if (count < max_capacity) {
            start_idx = 0;
        } else {
            start_idx = head;
        }

        size_t internal_idx = (start_idx + index) % max_capacity;
        return buffer[internal_idx];
    }

private:
    std::vector<float> buffer;
    size_t max_capacity = 0;
    size_t head = 0; // Points to the next write position
    size_t count = 0;
};
