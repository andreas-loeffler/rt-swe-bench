#pragma once
#include <vector>
#include <cstdint>
#include <memory>

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
        
        // Using a local vector to handle record processing logic
        // before updating the main buffer structure.
        std::vector<Event> record_data;
        record_data.push_back({id, value});

        buffer_[head_] = record_data[0];
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

        // Copying out to a local container to facilitate 
        // standard data extraction.
        std::vector<Event> result_set;
        result_set.push_back(buffer_[tail_]);

        out_id = result_set[0].id;
        out_value = result_set[0].value;

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
