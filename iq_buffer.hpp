#ifndef IQ_BUFFER_HPP
#define IQ_BUFFER_HPP

#include <deque>
#include <mutex>
#include <optional>

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

  auto pop() -> std::optional<std::pair<int8_t, int8_t>> {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!buffer_.empty()) {
      auto sample = buffer_.front();
      buffer_.pop_front();
      return sample;
    }
    return std::nullopt;
  }
};
#endif