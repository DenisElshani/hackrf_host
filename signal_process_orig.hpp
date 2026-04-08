#ifndef SIGNAL_PROCESS_HPP
#define SIGNAL_PROCESS_HPP

#include "iq_buffer.hpp"

#include <hackrf.h> 
#include <iostream>
#include <vector>
#include <bitset> 
#include <numeric>

#define RX_BIT_SAMPLES 1000
#define RX_THRESHOLD_MARGIN 50


class SignalProcessor {
private:
  std::vector<float> _lut; // Lookup table for converting int8 to float
  uint32_t sample_count = 0;
  uint32_t mag2_sum = 0;
  uint32_t noise_floor = 0;

  // Variables for samples window alignment
  uint32_t mag2_window[RX_BIT_SAMPLES];
  uint32_t window_index = 0;
    bool synced = false;
  
  uint8_t preamble_shift = 0;
  uint8_t preamble_window_detector = 0;

  // State Machine 
  typedef enum {
    STATE_IDLE,        // Looking for 10101010 pattern
    STATE_REFINE_SYNC, // Pattern found, now finding the exact peak
    STATE_LOCKED       // Synchronized, reading data bits
  } rx_state_t;

  rx_state_t state = STATE_IDLE;
  uint8_t pattern_shift = 0;
  uint32_t sample_in_bit_counter = 0;

  // Integration variables
  uint64_t sliding_sum = 0;
  uint32_t energy_window[RX_BIT_SAMPLES] = {0}; // RX_BIT_SAMPLES samples
  uint32_t window_ptr = 0;
  uint64_t max_energy_found = 0;
  uint32_t samples_since_max = 0;
  uint32_t circular_buffer = 0;
  uint32_t synchronization_counter = 0;


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
      mag2_sum += mag2;
      sample_count++;
      std::cout << mag2 <<";";
          if (sample_count >= RX_BIT_SAMPLES) {
            std::cout <<";;";
            const uint32_t mag2_avg = mag2_sum / RX_BIT_SAMPLES;
            noise_floor = (noise_floor * 15 + mag2_avg) / 16;
            const uint32_t adaptive_threshold = noise_floor + RX_THRESHOLD_MARGIN;
            bool current_bit;
            current_bit = (mag2_avg > adaptive_threshold);
            
            // High-precision bit decision
            //bit_buffer[bit_buffer_index++] = current_bit;
            
            sample_count = 0;
            mag2_sum = 0;
          }
      }
    
    }
};

#endif