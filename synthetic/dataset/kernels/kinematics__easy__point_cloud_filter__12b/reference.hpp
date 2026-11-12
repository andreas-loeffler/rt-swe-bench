#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

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
        size_t count_passed = 0;

        for (size_t i = 0; i < to_process; ++i) {
            float x = input_points[i * 3];
            float y = input_points[i * 3 + 1];
            float z = input_points[i * 3 + 2];

            if ((x * x + y * y + z * z) <= threshold_sq_) {
                output_points[count_passed * 3] = x;
                output_points[count_passed * 3 + 1] = y;
                output_points[count_passed * 3 + 2] = z;
                count_passed++;
            }
        }
        return count_passed;
    }

private:
    size_t max_points_ = 0;
    float threshold_sq_ = 0.0f;
    std::vector<float> buffer_;
};
