#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <algorithm>

struct TelemetryEvent {
    uint32_t id;
    double timestamp;
    float value;

    bool operator==(const TelemetryEvent&) const = default;
};

class FixedCapacityEventQueue {
public:
    void configure(size_t capacity) {
        buffer_.assign(capacity + 1, {0, 0.0, 0.0});
        capacity_ = capacity;
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(const TelemetryEvent& event) {
        return process_push(event);
    }

    bool pop(TelemetryEvent& event) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);

        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        event = buffer_[current_tail];
        tail_.store((current_tail + 1) % buffer_.size(), std::memory_order_release);
        return true;
    }

private:
    bool process_push(const TelemetryEvent& event) {
        std::vector<TelemetryEvent> event_copy = {event};
        
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % buffer_.size();

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[current_head] = event_copy[0];
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    std::vector<TelemetryEvent> buffer_;
    size_t capacity_ = 0;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};
