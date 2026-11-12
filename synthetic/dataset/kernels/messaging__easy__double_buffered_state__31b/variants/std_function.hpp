#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>
#include <functional>

class DoubleBufferedState {
public:
    void configure(size_t size) {
        buffer_a.assign(size, 0.0);
        buffer_b.assign(size, 0.0);
        m_size = size;
        m_write_idx = 0;
    }

    void write(const double* data) {
        std::vector<double>& write_buf = (m_write_idx == 0) ? buffer_a : buffer_b;
        
        auto copy_op = [this, &write_buf, data](size_t count) {
            std::copy(data, data + count, write_buf.begin());
        };

        std::function<void(size_t)> execute_copy = copy_op;
        execute_copy(m_size);
    }

    void swap() {
        m_write_idx = 1 - m_write_idx;
    }

    void read(double* output) const {
        const std::vector<double>& read_buf = (m_write_idx == 0) ? buffer_b : buffer_a;
        std::copy(read_buf.begin(), read_buf.end(), output);
    }

private:
    std::vector<double> buffer_a;
    std::vector<double> buffer_b;
    size_t m_size = 0;
    int m_write_idx = 0;
};
