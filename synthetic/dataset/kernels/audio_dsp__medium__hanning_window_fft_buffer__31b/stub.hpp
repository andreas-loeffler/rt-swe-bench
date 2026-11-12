#pragma once
#include <cstddef>

class WindowedBuffer {
public:
    void configure(size_t frame_size) {
        // TODO: Allocate buffers and pre-calculate Hanning window
    }

    bool push(float sample) {
        // TODO: Accumulate sample and return true when frame is full
        return false;
    }

    const float* get_frame() {
        // TODO: Return pointer to windowed frame
        return nullptr;
    }
};
