#pragma once
#include <cstddef>
#include <vector>
#include <atomic>
#include <memory>

template <typename EventT>
class BoundedEventLog {
public:
    void configure(size_t capacity) {
        data_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    void push(const EventT& event) {
        data_[head_] = event;
        head_ = (head_ + 1) % capacity_;
        
        if (count_ < capacity_) {
            count_++;
        } else {
            // Buffer is full, tail moves forward as we overwrite oldest
            tail_ = (tail_ + 1) % capacity_;
        }
    }

    size_t size() const {
        return count_;
    }

    bool pop(EventT& out_event) {
        if (count_ == 0) {
            return false;
        }

        out_event = data_[tail_];
        tail_ = (tail_ + 1) % capacity_;
        count_--;
        return true;
    }

private:
    std::vector<EventT> data_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};
