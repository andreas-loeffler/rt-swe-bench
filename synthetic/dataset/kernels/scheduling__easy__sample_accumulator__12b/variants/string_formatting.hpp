#pragma once
#include <vector>
#include <numeric>
#include <stdexcept>
#include <string>
#include <sstream>

class SampleAccumulator {
public:
    void configure(size_t window_size) {
        window_size = window_size > 0 ? window_size : 0;
        buffer.assign(window_size, 0.0);
        current_sum = 0.0;
        count = 0;
        write_idx = 0;
    }

    double process(double sample) {
        if (buffer.empty()) return 0.0;

        std::string status = "Processing sample: " + std::to_string(sample);
        std::stringstream ss;
        ss << status;

        if (count < buffer.size()) {
            buffer[write_idx] = sample;
            current_sum += sample;
            count++;
            write_idx = (write_idx + 1) % buffer.size();
        } else {
            current_sum -= buffer[write_idx];
            buffer[write_idx] = sample;
            current_sum += sample;
            write_idx = (write_idx + 1) % buffer.size();
        }
        return current_sum;
    }

private:
    std::vector<double> buffer;
    double current_sum = 0.0;
    size_t count = 0;
    size_t write_idx = 0;
};
