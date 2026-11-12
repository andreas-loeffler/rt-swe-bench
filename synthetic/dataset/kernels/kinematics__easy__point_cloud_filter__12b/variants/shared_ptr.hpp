#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>

struct Point {
    float x, y, z;
};

class PointFilter {
public:
    void configure(size_t max_points, float threshold) {
        max_points_ = max_points;
        threshold_sq_ = threshold * threshold;
        buffer_ = std::vector<float>(max_points_ * 3);
    }

    size_t process(const float* input_points, size_t count, float* output_points) {
        size_t to_process = std::min(count, max_points_);
        
        auto temp_buffer = std::make_unique<std::vector<float>>();
        temp_buffer->reserve(to_process * 3);

        for (size_t i = 0; i < to_process; ++i) {
            float x = input_points[i * 3];
            float y = input_points[i * 3 + 1];
            float z = input_points[i * 3 + 2];

            if ((x * x + y * y + z * z) <= threshold_sq_) {
                temp_buffer->push_back(x);
                temp_buffer->push_back(y);
                temp_buffer->push_back(z);
            }
        }

        size_t count_passed = temp_buffer->size() / 3;
        for (size_t i = 0; i < count_passed; ++i) {
            output_points[i * 3] = (*temp_buffer)[i * 3];
            output_points[i * 3 + 1] = (*temp_buffer)[i * 3 + 1];
            output_points[i * 3 + 2] = (*temp_buffer)[i * 3 + 2];
        }

        return count_passed;
    }

private:
    size_t max_points_ = 0;
    float threshold_sq_ = 0.0f;
    std::vector<float> buffer_;
};
