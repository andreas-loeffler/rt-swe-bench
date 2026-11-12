#pragma once
#include <cstddef>

template <typename EventT>
class BoundedEventLog {
public:
    void configure(size_t capacity) {
        // TODO: Implement allocation
    }

    void push(const EventT& event) {
        // TODO: Implement real-time push
    }

    size_t size() const {
        // TODO: Implement size retrieval
        return 0;
    }

    bool pop(EventT& out_event) {
        // TODO: Implement real-time pop
        return false;
    }
};
