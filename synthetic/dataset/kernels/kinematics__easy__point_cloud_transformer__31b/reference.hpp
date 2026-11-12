#pragma once

#include <vector>
#include <cstddef>

class PointCloudTransformer {
public:
    void configure(size_t max_points) {
        // Pre-allocate internal buffer if needed for intermediate calculations
        // though for this specific kernel, we can operate directly on output_points.
        m_max_points = max_points;
        m_buffer.resize(max_points * 3);
    }

    void transform(const float* input_points, float* output_points, const float* transform_matrix, size_t num_points) {
        if (num_points > m_max_points) return;

        for (size_t i = 0; i < num_points; ++i) {
            const float x = input_points[i * 3 + 0];
            const float y = input_points[i * 3 + 1];
            const float z = input_points[i * 3 + 2];

            // transform_matrix is row-major: 
            // [0 1 2 3]
            // [4 5 6 7]
            // [8 9 10 11]
            // [12 13 14 15]
            output_points[i * 3 + 0] = transform_matrix[0] * x + transform_matrix[1] * y + transform_matrix[2] * z + transform_matrix[3];
            output_points[i * 3 + 1] = transform_matrix[4] * x + transform_matrix[5] * y + transform_matrix[6] * z + transform_matrix[7];
            output_points[i * 3 + 2] = transform_matrix[8] * x + transform_matrix[9] * y + transform_matrix[10] * z + transform_matrix[11];
        }
    }

private:
    size_t m_max_points = 0;
    std::vector<float> m_buffer;
};
