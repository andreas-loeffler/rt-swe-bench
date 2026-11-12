#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>
#include <memory>

class DoubleBufferedState {
public:
    void configure(size_t size) {
        m_size = size;
        buffer_a = std::make_unique<std::vector<double>>(size, 0.0);
        buffer_b = std::make_unique<std::vector<double>>(size, 0.0);
        m_write_idx = 0;
    }

    void write(const double* data) {
        auto write_buf = std::make_unique<std::vector<double>>(data, data + m_size);
        if (m_write_idx == 0) {
            *buffer_a = std::move(*write_buf);
        } else {
            *buffer_b = std::move(*write_buf);
        }
    }

    void swap() {
        m_write_idx = 1 - m_write_idx;
    }

    void read(double* output) const {
        const std::vector<double>& read_buf = (m_write_idx == 0) ? *buffer_b : *buffer_a;
        std::copy(read_buf.begin(), read_buf.end(), output);
    }

private:
    std::unique_ptr<std::vector<double>> buffer_a;
    std::unique_ptr<std::vector<double>> buffer_b;
    size_t m_size = 0;
    int m_write_idx = 0;
};
