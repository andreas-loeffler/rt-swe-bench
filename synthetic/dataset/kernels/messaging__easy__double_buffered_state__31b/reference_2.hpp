#pragma once
#include <cstddef>
#include <memory>
#include <algorithm>

class DoubleBufferedState {
public:
    void configure(size_t size) {
        bufferSize_ = size;
        buf[0] = std::make_unique<double[]>(size);
        buf[1] = std::make_unique<double[]>(size);
        writeIdx_ = 1;
    }

    void write(const double* data) {
        for (size_t i = 0; i < bufferSize_; ++i) {
            buf[writeIdx_][i] = data[i];
        }
    }

    void swap() {
        writeIdx_ = (writeIdx_ == 0) ? 1 : 0;
    }

    void read(double* output) {
        const size_t readIdx = (writeIdx_ == 0) ? 1 : 0;
        for (size_t i = 0; i < bufferSize_; ++i) {
            output[i] = buf[readIdx][i];
        }
    }

private:
    size_t bufferSize_ = 0;
    std::unique_ptr<double[]> buf[2];
    size_t writeIdx_ = 1;
};
