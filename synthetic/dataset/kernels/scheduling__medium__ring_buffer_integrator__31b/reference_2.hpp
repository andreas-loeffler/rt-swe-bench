#pragma once
#include <cstddef>
#include <vector>
#include <numeric>

class RingBufferIntegrator {
public:
    void configure(size_t window_size) {
        buffer.resize(window_size);
        current_sum = 0.0f;
        head = 0;
        count = 0;
    }

    void push(float value) {
        if (buffer.empty()) return;

        if (count < buffer.size()) {
            // Buffer is still filling up
            buffer[count] = value;
            current_sum += value;
            count++;
            if (count == buffer.size()) {
                head = 0;
            }
        } else {
            // Buffer is full, replace oldest
            current_sum -= buffer[head];
            buffer[head] = value;
            current_sum += value;
            head = (head + 1) % buffer.size();
        }
    }

    float get_sum() const {
        return current_sum;
    }

    void reset() {
        // Clear state without reallocating
        count = 0;
        head = 0;
        current_sum = 0.0f;
        // Note: We don't need to zero the buffer as count handles "emptiness"
    }

private:
    std::vector<float> buffer;
    float current_sum = 0.0f;
    size_t head = 0;
    size_t count = 0;
};
