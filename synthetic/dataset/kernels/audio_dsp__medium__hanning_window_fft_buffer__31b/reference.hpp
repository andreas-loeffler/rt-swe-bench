#pragma once
#include <vector>
#include <cmath>
#include <numbers>

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
            // Apply window in-place for the frame to be read by get_frame
            // Note: In a real system, we might double-buffer, but for this kernel 
            // we apply the window to the filled buffer.
            for (size_t i = 0; i < m_frame_size; ++i) {
                m_buffer[i] *= m_window[i];
            }
            return true;
        }
        return false;
    }

    const float* get_frame() const {
        return m_buffer.data();
    }

private:
    size_t m_frame_size = 0;
    size_t m_write_pos = 0;
    std::vector<float> m_buffer;
    std::vector<float> m_window;
};
