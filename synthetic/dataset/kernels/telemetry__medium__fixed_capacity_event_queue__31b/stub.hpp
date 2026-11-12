#pragma once

#include <vector>
#include <atomic>
#include <cstddef>

struct TelemetryEvent {
    uint32_t id;
    double timestamp;
    float value;

    bool operator==(const TelemetryEvent&) const = default;
};

class FixedCapacityEventQueue {
public:
    void configure(size_t capacity) {
        // TODO: Implement allocation
    }

    bool push(const TelemetryEvent& event) {
        // TODO: Implement lock-free push
        return false;
    }

    bool pop(TelemetryEvent& event) {
        // TODO: Implement lock-free pop
        return false;
    }

private:
    std::vector<TelemetryEvent> buffer_;
    size_t capacity_ = 0;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};
