#pragma once
#include <vector>
#include <cstdint>

struct Event {
    uint32_t id;
    float value;
};

class CircularLog {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        buffer_.clear();
        buffer_.resize(capacity);
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    bool push(uint32_t id, float value) {
        if (capacity_ == 0) return false;
        
        buffer_[head_] = {id, value};
        head_ = (head_ + 1) % capacity_;
        
        if (count_ < capacity_) {
            count_++;
        } else {
            tail_ = (tail_ + 1) % capacity_;
        }
        return true;
    }

    bool pop(uint32_t& out_id, float& out_value) {
        if (count_ == 0) return false;

        Event& e = buffer_[tail_];
        out_id = e.id;
        out_value = e.value;
        tail_ = (tail_ + 1) % capacity_;
        count_--;
        return true;
    }

    size_t size() const {
        return count_;
    }

private:
    std::vector<Event> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};
