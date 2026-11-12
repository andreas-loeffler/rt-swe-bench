#pragma once
#include <vector>
#include <numeric>

class DeadReckoningPos {
public:
    /**
     * Configures the kernel with the number of previous velocity measurements to consider.
     * Called during initialization; may allocate memory.
     */
    void configure(size_t window_size) {
        // Reserve memory to avoid allocations in update()
        buffer.assign(window_size, 0.0);
        
        capacity = window_size;
        head = 0;
        count = 0;
        running_sum = 0.0;
    }

    /**
     * Adds a new velocity measurement to the history.
     * Real-time: No allocation, no locking, no I/O.
     */
    void update(double velocity) {
        if (capacity == 0) return;

        // If buffer is full, we subtract the element being overwritten from the sum
        if (count == capacity) {
            running_sum -= buffer[head];
        } else {
            count++;
        }

        buffer[head] = velocity;
        running_sum += velocity;

        // Advance head pointer in circular fashion
        head = (head + 1) % capacity;
    }

    /**
     * Calculates the current estimated position.
     * Real-time: No allocation, no locking, no I/O.
     */
    double get_position() const {
        return running_sum;
    }

private:
    std::vector<double> buffer;
    size_t capacity = 0;
    size_t head = 0;
    size_t count = 0;
    double running_sum = 0.0;
};
