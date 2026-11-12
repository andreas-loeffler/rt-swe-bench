#pragma once

#include <cstddef>
#include <vector>

class PointCloudTransformer {
public:
    void configure(size_t max_points) {
        m_max_points = max_points;
        // Pre-allocate internal buffer if needed. 
        // Based on the API, we don't strictly need a buffer unless we 
        // want to store points, but we must ensure we don't allocate in transform.
        m_points_buffer.reserve(m_max_points * 3);
    }

    void transform(const float* input_points, float* output_points, const float* transform_matrix, size_t num_points) {
        if (num_points > m_max_points) return;

        // Matrix is row-major:
        // m00 m01 m02 m03
        // m10 m11 m12 m13
        // m20 m21 m22 m23
        // m30 m31 m32 m33
        
        for (size_t i = 0; i < num_points; ++i) {
            float x = input_points[i * 3 + 0];
            float y = input_points[i * 3 + 1];
            float z = input_points[i * 3 + 2];

            // P_world = M * [x, y, z, 1]^T
            // x' = m00*x + m01*y + m02*z + m03*1
            // y' = m10*x + m11*y + m12*z + m13*1
            // z' = m20*x + m21*y + m22*z + m23*1
            
            output_points[i * 3 + 0] = transform_matrix[0] * x + transform_matrix[1] * y + 
                                         transform_matrix[2] * z + transform_matrix[3];
            
            output_points[i * 3 + 1] = transform_matrix[4] * x + transform_matrix[5] * y + 
                                         transform_matrix[6] * z + transform_matrix[7];
            
            output_points[i * 3 + 2] = transform_matrix[8] * x + transform_matrix[9] * y + 
                                         transform_matrix[10] * z + transform_matrix[11];
        }
    }

private:
    size_t m_max_points = 0;
    std::vector<float> m_points_buffer;
};
