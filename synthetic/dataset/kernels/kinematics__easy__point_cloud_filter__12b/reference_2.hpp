#pragma once
#include <cstddef>
#include <vector>

class PointFilter {
public:
    /**
     * Configures the filter with the maximum number of points to process and the distance threshold.
     * This is called during the initialization phase and may allocate memory.
     */
    void configure(size_t max_points, float threshold) {
        m_max_points = max_points;
        m_threshold_sq = threshold * threshold;
        
        // Although the current process implementation writes directly to the provided 
        // output_points buffer, we reserve space if the logic were to ever require 
        // internal temporary buffering for the filtered points.
        m_internal_buffer.reserve(max_points * 3);
    }

    /**
     * Processes a buffer of 3D points.
     * Hard real-time constraint: no allocations, no locks, no throws, no I/O.
     */
    size_t process(const float* input_points, size_t count, float* output_points) {
        if (input_points == nullptr || output_points == nullptr || count == 0) {
            return 0;
        }

        // Limit processing to the maximum points configured during initialization
        const size_t points_to_process = (count < m_max_points) ? count : m_max_points;
        size_t passed_count = 0;

        for (size_t i = 0; i < points_to_process; ++i) {
            const size_t offset = i * 3;
            const float x = input_points[offset];
            const float y = input_points[offset + 1];
            const float z = input_points[offset + 2];

            // Use squared distance to avoid expensive sqrt() call
            const float dist_sq = (x * x) + (y * y) + (z * z);

            if (dist_sq <= m_threshold_sq) {
                const size_t out_offset = passed_count * 3;
                output_points[out_offset] = x;
                output_points[out_offset + 1] = y;
                output_points[out_offset + 2] = z;
                ++passed_count;
            }
        }

        return passed_count;
    }

private:
    size_t m_max_points = 0;
    float m_threshold_sq = 0.0f;
    std::vector<float> m_internal_buffer;
};
