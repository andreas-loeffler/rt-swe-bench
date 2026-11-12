#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

class SortedWindow {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        buffer_.reserve(capacity);
        buffer_.clear();
        insertion_order_.reserve(capacity);
    }

    double process(double value) {
        if (buffer_.size() < capacity_) {
            auto it = std::lower_bound(buffer_.begin(), buffer_.end(), value);
            buffer_.insert(it, value);
            insertion_order_.push_back(value);
        } else {
            double oldest = insertion_order_.front();
            insertion_order_.erase(insertion_order_.begin());
            
            auto it = std::lower_bound(buffer_.begin(), buffer_.end(), oldest);
            if (it != buffer_.end() && *it == oldest) buffer_.erase(it);
            
            auto it_new = std::lower_bound(buffer_.begin(), buffer_.end(), value);
            buffer_.insert(it_new, value);
            insertion_order_.push_back(value);
        }

        // Use a dynamic container for result calculation to handle intermediate steps
        auto result_data = std::make_shared<std::vector<double>>();
        result_data->assign(buffer_.begin(), buffer_.end());
        
        size_t n = result_data->size();
        if (n % 2 == 1) {
            return (*result_data)[n / 2];
        } else {
            return ((*result_data)[n / 2 - 1] + (*result_data)[n / 2]) / 2.0;
        }
    }

private:
    size_t capacity_;
    std::vector<double> buffer_;
    std::vector<double> insertion_order_;
};
