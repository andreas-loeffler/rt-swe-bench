#pragma once
#include <vector>

class TimeoutMonitor {
public:
    void configure(size_t window_size, double threshold_ms) {
        // TODO
    }

    bool push(double timestamp_ms) {
        // TODO
        return false;
    }

    bool check_timeout() {
        // TODO
        return false;
    }

private:
    // TODO
};
