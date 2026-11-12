#pragma once
#include <vector>
#include <cstddef>

template <typename EventT>
class BoundedEventLog {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        max_size_ = capacity;
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    void push(const EventT& event) {
        buffer_[head_] = event;
        head_ = (head_ + 1) % max_size_;
        if (count_ < max_size_) {
            count_++;
        } else {
            tail_ = (tail_ + 1) % max_size_;
        }
    }

    size_t size() const {
        return count_;
    }

    bool pop(EventT& out_event) {
        if (count_ == 0) {
            return false;
        }
        out_event = buffer_[tail_];
        tail_ = (tail_ + 1) % max_size_;
        count_--;
        return true;
    }

private:
    std::vector<EventT> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};
