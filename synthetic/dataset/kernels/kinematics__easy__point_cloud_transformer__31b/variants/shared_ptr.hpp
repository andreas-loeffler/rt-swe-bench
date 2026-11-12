#pragma once

#include <vector>
#include <cstddef>
#include <memory>

class PointCloudTransformer {
public:
    void configure(size_t max_points) {
        m_max_points = max_points;
    }

    void transform(const float* input_points, float* output_points, const float* transform_matrix, size_t num_points) {
        if (num_points > m_max_points) return;

        auto context = std::make_unique<TransformContext>();
        context->matrix = transform_matrix;

        for (size_t i = 0; i < num_points; ++i) {
            const float x = input_points[i * 3 + 0];
            const float y = input_points[i * 3 + 1];
            const float z = input_points[i * 3 + 2];

            const float* m = context->matrix;
            output_points[i * 3 + 0] = m[0] * x + m[1] * y + m[2] * z + m[3];
            output_points[i * 3 + 1] = m[4] * x + m[5] * y + m[6] * z + m[7];
            output_points[i * 3 + 2] = m[8] * x + m[9] * y + m[10] * z + m[11];
        }
    }

private:
    struct TransformContext {
        const float* matrix;
    };

    size_t m_max_points = 0;
};
