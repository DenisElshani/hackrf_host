#ifndef IQ_BUFFER_HPP
#define IQ_BUFFER_HPP

#include <deque>
#include <mutex>
#include <optional>
#include <vector>
#include <algorithm>
#include <stdexcept>

class IQBuffer {
private:
    //std::deque<std::pair<int8_t, int8_t>> buffer_;
    std::mutex mutex_;
    size_t capacity_;

    std::vector<std::pair<int8_t, int8_t>> buffer_;
    size_t head_ = 0; // Where we write
    size_t tail_ = 0; // Where we read
    size_t full_count_ = 0;

    public:
    // Keep capacity reasonable (e.g., 10,000,000)
    IQBuffer(size_t capacity) : capacity_(capacity) {
        if (capacity_ < 1024) capacity_ = 1024; 
        buffer_.resize(capacity_);
    }

    double get_utilization_percent() {
        std::lock_guard<std::mutex> lock(mutex_);
        return (static_cast<double>(full_count_) / capacity_) * 100.0;
    }
    
    size_t get_count() {
        std::lock_guard<std::mutex> lock(mutex_);
        return full_count_;
    }

    void push_block(const uint8_t* raw_samples, size_t num_samples) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (size_t i = 0; i < num_samples; i++) {
            // Cast here so the math (I*I + Q*Q) treats them as signed values
            int8_t I = static_cast<int8_t>(raw_samples[2 * i]);
            int8_t Q = static_cast<int8_t>(raw_samples[2 * i + 1]);
            
            buffer_[head_] = {I, Q};
            head_ = (head_ + 1) % capacity_;
            
            if (full_count_ < capacity_) {
                full_count_++;
            } else {
                // Overwriting: Move tail forward to stay valid
                tail_ = (tail_ + 1) % capacity_;
            }
        }
    }

    auto pop_batch(size_t max_request) -> std::vector<std::pair<int8_t, int8_t>> {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Ensure we don't ask for more than we have
        size_t to_copy = std::min(max_request, full_count_);
        if (to_copy == 0) return {};

        std::vector<std::pair<int8_t, int8_t>> result;
        result.reserve(to_copy);

        for (size_t i = 0; i < to_copy; ++i) {
            result.push_back(buffer_[tail_]);
            tail_ = (tail_ + 1) % capacity_;
        }
        
        full_count_ -= to_copy;
        return result;
    }
};

#endif