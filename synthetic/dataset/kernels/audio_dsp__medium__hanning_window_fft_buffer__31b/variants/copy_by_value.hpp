#pragma once
#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

class WindowedBuffer {
public:
    void configure(size_t frame_size) {
        m_frame_size = frame_size;
        m_buffer.assign(frame_size, 0.0f);
        m_window.assign(frame_size, 0.0f);
        
        for (size_t n = 0; n < m_frame_size; ++n) {
            if (m_frame_size > 1) {
                m_window[n] = 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * n / (m_frame_size - 1)));
            } else {
                m_window[n] = 1.0f;
            }
        }
        m_write_pos = 0;
    }

    bool push(float sample) {
        m_buffer[m_write_pos++] = sample;
        
        if (m_write_pos >= m_frame_size) {
            m_write_pos = 0;
            
            std::vector<float> windowed_frame = apply_window(m_buffer, m_window);
            m_buffer = windowed_frame;
            
            return true;
        }
        return false;
    }

    const float* get_frame() const {
        return m_buffer.data();
    }

private:
    std::vector<float> apply_window(const std::vector<float> input, const std::vector<float> window) {
        std::vector<float> output(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = input[i] * window[i];
        }
        return output;
    }

    size_t m_frame_size = 0;
    size_t m_write_pos = 0;
    std::vector<float> m_buffer;
    std::vector<float> m_window;
};
