#ifndef IQ_BUFFER_HPP
#define IQ_BUFFER_HPP

#include <deque>
#include <mutex>
#include <optional>
#include <vector>

class IQBuffer {
private:
  std::deque<std::pair<int8_t, int8_t>> buffer_;
  std::mutex mutex_;
  size_t capacity_;

public:
  IQBuffer(size_t capacity) : capacity_(capacity) {}

  void push(int8_t I, int8_t Q) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_.size() < capacity_) {
      buffer_.emplace_back(I, Q);
    } else {
      // Handle overflow (e.g., drop oldest sample)
      buffer_.pop_front();
      buffer_.emplace_back(I, Q);
    }
  }

  void push_block(uint8_t* raw_samples, size_t num_samples) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (size_t i = 0; i < num_samples; i++) {
      int8_t I = raw_samples[2 * i];
      int8_t Q = raw_samples[2 * i + 1];
      
      if (buffer_.size() < capacity_) {
          buffer_.emplace_back(I, Q);
      } else {
          // Drop oldest to make room
          buffer_.pop_front();
          buffer_.emplace_back(I, Q);
      }
    }
  } 

  auto pop() -> std::optional<std::pair<int8_t, int8_t>> {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!buffer_.empty()) {
      auto sample = buffer_.front();
      buffer_.pop_front();
      return sample;
    }
    return std::nullopt;
  }

  auto pop_batch(size_t max_samples) -> std::vector<std::pair<int8_t, int8_t>> {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<int8_t, int8_t>> batch;

    // Determine how many samples we can actually take
    size_t count = std::min(max_samples, buffer_.size());
    
    if (count > 0) {
        batch.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            batch.push_back(buffer_.front());
            buffer_.pop_front();
        }
    }
    return batch; // Returns an empty vector if no data is available
  }

};
#endif