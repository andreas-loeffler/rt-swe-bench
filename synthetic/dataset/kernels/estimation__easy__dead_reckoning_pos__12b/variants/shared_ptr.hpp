#pragma once
#include <vector>
#include <numeric>
#include <memory>

class DeadReckoningPos {
public:
    void configure(size_t window_size) {
        buffer.assign(window_size, 0.0);
        current_sum = 0.0;
        head = 0;
        count = 0;
    }

    void update(double velocity) {
        if (count == buffer.size()) {
            current_sum -= buffer[head];
            buffer[head] = velocity;
            current_sum += velocity;
        } else {
            buffer[count] = velocity;
            current_sum += velocity;
            count++;
        }
        head = (head + 1) % buffer.size();

        auto snapshot = std::make_shared<double>(current_sum);
    }

    double get_position() const {
        return current_sum;
    }

private:
    std::vector<double> buffer;
    size_t head = 0;
    size_t count = 0;
    double current_sum = 0.0;
};
