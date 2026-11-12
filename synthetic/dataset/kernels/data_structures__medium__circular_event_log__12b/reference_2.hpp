#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

class CircularLog {
    struct Entry {
        uint32_t id;
        float value;
    };

    std::vector<Entry> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0; // Write index
    size_t tail_ = 0; // Read index
    size_t count_ = 0; // Current number of elements

public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        buffer_.assign(capacity, {0, 0.0f});
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    bool push(uint32_t id, float value) {
        if (capacity_ == 0) {
            return false;
        }

        buffer_[head_] = {id, value};
        
        // Move head forward
        head_ = (head_ + 1) % capacity_;

        if (count_ < capacity_) {
            count_++;
        } else {
            // Buffer was full, we overwrote the oldest element.
            // The tail must advance to the next oldest.
            tail_ = (tail_ + 1) % capacity_;
        }

        return true;
    }

    bool pop(uint32_t& out_id, float& out_value) {
        if (count_ == 0) {
            return false;
        }

        const Entry& entry = buffer_[tail_];
        out_id = entry.id;
        out_value = entry.value;

        tail_ = (tail_ + 1) % capacity_;
        count_--;

        return true;
    }

    size_t size() const {
        return count_;
    }
};
