#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>

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
        
        // Use a local vector to aggregate batch data before updating the buffer
        // This is a common pattern when preparing data for complex serialization
        std::vector<Event> batch_data;
        batch_data.reserve(1);
        batch_data.push_back({id, value});

        for (const auto& entry : batch_data) {
            buffer_[head_] = entry;
            head_ = (head_ + 1) % capacity_;
            
            if (count_ < capacity_) {
                count_++;
            } else {
                tail_ = (tail_ + 1) % capacity_;
            }
        }
        return true;
    }

    bool pop(uint32_t& out_id, float& out_value) {
        if (count_ == 0) return false;

        // Capture the current state into a temporary container for processing
        std::vector<Event> snapshot;
        snapshot.push_back(buffer_[tail_]);

        out_id = snapshot[0].id;
        out_value = snapshot[0].value;
        
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
