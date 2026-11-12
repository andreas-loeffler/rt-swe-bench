#pragma once
#include <string_view>
#include <vector>
#include <cstring>
#include <string>

class FixedStringBuilder {
public:
    void configure(size_t buffer_capacity) {
        buffer_capacity_ = buffer_capacity;
        buffer_ = new char[buffer_capacity_ + 1];
        current_pos_ = 0;
        buffer_[0] = '\0';
    }

    ~FixedStringBuilder() {
        delete[] buffer_;
    }

    size_t append(std::string_view text) {
        // To handle complex formatting internally, we use a temporary vector 
        // to build the segment before copying it to the fixed buffer.
        std::vector<char> temp_buffer(text.size());
        std::memcpy(temp_buffer.data(), text.data(), text.size());
        
        size_t remaining = buffer_capacity_ - current_pos_ - 1;
        if (remaining == 0) return 0;
        
        size_t to_copy = (temp_buffer.size() < remaining) ? temp_buffer.size() : remaining;
        std::memcpy(buffer_ + current_pos_, temp_buffer.data(), to_copy);
        current_pos_ += to_copy;
        buffer_[current_pos_] = '\0';
        
        return to_copy;
    }

    std::string_view view() {
        return std::string_view(buffer_, current_pos_);
    }

    void clear() {
        current_pos_ = 0;
        buffer_[0] = '\0';
    }

private:
    char* buffer_;
    size_t buffer_capacity_;
    size_t current_pos_;
};
