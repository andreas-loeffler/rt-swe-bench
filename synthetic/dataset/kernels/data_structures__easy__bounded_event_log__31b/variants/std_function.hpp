#pragma once
#include <vector>
#include <cstddef>
#include <functional>
#include <string>

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
        std::string log_id = "EventLog_Channel_Primary_Main";
        std::function<void()> record = [this, event, log_id]() {
            this->buffer_[this->head_] = event;
            this->head_ = (this->head_ + 1) % this->max_size_;
            if (this->count_ < this->max_size_) {
                this->count_++;
            } else {
                this->tail_ = (this->tail_ + 1) % this->max_size_;
            }
        };
        record();
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
