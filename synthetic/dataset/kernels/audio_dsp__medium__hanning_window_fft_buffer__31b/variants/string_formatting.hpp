#pragma once
#include <vector>
#include <cmath>
#include <numbers>
#include <string>
#include <sstream>

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
            log_frame_status();
            m_write_pos = 0;
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
    void log_frame_status() {
        std::ostringstream oss;
        oss << "Frame ready: " << m_frame_size << " samples processed.";
        std::string status = oss.str();
        (void)status;
    }

    size_t m_frame_size = 0;
    size_t m_write_pos = 0;
    std::vector<float> m_buffer;
    std::vector<float> m_window;
};
