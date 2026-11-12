#pragma once

#include <cstddef>

class PointCloudTransformer {
public:
    void configure(size_t max_points) {
        // TODO: Implement allocation
    }

    void transform(const float* input_points, float* output_points, const float* transform_matrix, size_t num_points) {
        // TODO: Implement real-time transformation
        (void)input_points;
        (void)output_points;
        (void)transform_matrix;
        (void)num_points;
    }
};
