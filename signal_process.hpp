#ifndef SIGNAL_PROCESS_HPP
#define SIGNAL_PROCESS_HPP

#include "iq_buffer.hpp"

#include <hackrf.h>
#include <iostream>
#include <vector>

#define RX_BIT_SAMPLES 1000
#define RX_THRESHOLD_MARGIN 50
class SignalProcessor {
private:
  std::vector<float> _lut; // Lookup table for converting int8 to float
  uint32_t sample_count_ = 0;
  uint32_t mag2_sum_ = 0;
  uint32_t noise_floor_ = 0;

  static std::pair<float, float> convert_sample(int8_t I, int8_t Q) {
    // From GNU Radio's gr-osmosdr
    // Convert int8 I/Q to float in range [-1.0, 1.0]
    return {float(I) * (1.0f / 128.0f), float(Q) * (1.0f / 128.0f)};
  }

public:
  SignalProcessor() {
    // initialise _lut
    for (unsigned int i = 0; i <= 0xff; i++) {
      _lut.push_back(float(int8_t(i)) * (1.0f / 128.0f));
    }
  }

  void process(IQBuffer &buffer) {
    if (const auto iq = buffer.pop(); iq) {
      // const auto [I, Q] = convert_sample(iq->first, iq->second);
      const uint32_t I = iq->first;
      const uint32_t Q = iq->second;
      const uint32_t mag2 = I * I + Q * Q;
      mag2_sum_ += mag2;
      std::cout << mag2 << "\t";
      sample_count_++;
      if (sample_count_ >= RX_BIT_SAMPLES) {
        const uint32_t mag2_avg = mag2_sum_ / RX_BIT_SAMPLES;
        noise_floor_ = (noise_floor_ * 15 + mag2_avg) / 16;
        const uint32_t adaptive_threshold = noise_floor_ + RX_THRESHOLD_MARGIN;
        const bool current_bit = (mag2_avg > adaptive_threshold);
        // std::cout << (current_bit ? '1' : '0');
        sample_count_ = 0;
        mag2_sum_ = 0;
      }
    }
  }
};

#endif