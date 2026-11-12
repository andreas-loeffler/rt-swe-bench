#pragma once
#include <deque>
#include <cstddef>

template <typename EventT>
class BoundedEventLog {
public:
    void configure(size_t capacity) {
        max_size_ = capacity;
        buffer_.clear();
    }

    void push(const EventT& event) {
        buffer_.push_back(event);
        if (buffer_.size() > max_size_) {
            buffer_.pop_front();
        }
    }

    size_t size() const {
        return buffer_.size();
    }

    bool pop(EventT& out_event) {
        if (buffer_.empty()) {
            return false;
        }
        out_event = buffer_.front();
        buffer_.pop_front();
        return true;
    }

private:
    std::deque<EventT> buffer_;
    size_t max_size_ = 0;
};
