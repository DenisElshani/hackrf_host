#ifndef SIGNAL_PROCESS_HPP
#define SIGNAL_PROCESS_HPP

#include "iq_buffer.hpp"

#include <hackrf.h>
#include <iostream>
#include <vector>

class SignalProcessor {
private:
  std::vector<float> _lut; // Lookup table for converting int8 to float

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
      const auto [I, Q] = convert_sample(iq->first, iq->second);
      const double mag2 = I * I + Q * Q;
      std::cout << mag2 << "\n";
    }
  }
};

#endif