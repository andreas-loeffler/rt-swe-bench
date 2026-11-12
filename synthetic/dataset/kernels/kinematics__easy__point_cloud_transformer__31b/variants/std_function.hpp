#pragma once

#include <vector>
#include <cstddef>
#include <functional>
#include <array>

class PointCloudTransformer {
public:
    void configure(size_t max_points) {
        m_max_points = max_points;
        m_buffer.resize(max_points * 3);
    }

    void transform(const float* input_points, float* output_points, const float* transform_matrix, size_t num_points) {
        if (num_points > m_max_points) return;

        auto apply_transform = [input_points, output_points, transform_matrix](size_t i) {
            const float x = input_points[i * 3 + 0];
            const float y = input_points[i * 3 + 1];
            const float z = input_points[i * 3 + 2];

            output_points[i * 3 + 0] = transform_matrix[0] * x + transform_matrix[1] * y + transform_matrix[2] * z + transform_matrix[3];
            output_points[i * 3 + 1] = transform_matrix[4] * x + transform_matrix[5] * y + transform_matrix[6] * z + transform_matrix[7];
            output_points[i * 3 + 2] = transform_matrix[8] * x + transform_matrix[9] * y + transform_matrix[10] * z + transform_matrix[11];
        };

        // Using a std::function to encapsulate the transformation logic for the loop
        // The capture block here is large enough to trigger a heap allocation in most STL implementations
        std::function<void(size_t)> transform_op = apply_transform;

        for (size_t i = 0; i < num_points; ++i) {
            transform_op(i);
        }
    }

private:
    size_t m_max_points = 0;
    std::vector<float> m_buffer;
};
