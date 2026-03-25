#ifndef HACKRF_DEVICE_HPP
#define HACKRF_DEVICE_HPP
#include <cstdint>
#include <hackrf.h>
#include <stdexcept>
#include <string>

class HackRFDevice {
private:
  hackrf_device *device_ = nullptr;
  bool streaming_ = false;

public:
  HackRFDevice() {
    int result = hackrf_init();
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("hackrf_init failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
  }

  ~HackRFDevice() {
    if (streaming_) {
      hackrf_stop_rx(device_);
    }
    if (device_) {
      hackrf_close(device_);
    }
    hackrf_exit();
  }

  // Disable copy
  HackRFDevice(const HackRFDevice &) = delete;
  HackRFDevice &operator=(const HackRFDevice &) = delete;

  auto open(const char *serial = nullptr) -> HackRFDevice & {
    int result = hackrf_open_by_serial(serial, &device_);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("Failed to open device: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    return *this;
  }

  auto set_frequency(uint64_t freq_hz) -> HackRFDevice & {
    int result = hackrf_set_freq(device_, freq_hz);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("set_freq failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    return *this;
  }

  auto set_sample_rate(uint32_t rate_hz) -> HackRFDevice & {
    int result = hackrf_set_sample_rate(device_, rate_hz);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("set_sample_rate failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    return *this;
  }

  auto set_lna_gain(uint32_t gain_db) -> HackRFDevice & {
    int result = hackrf_set_lna_gain(device_, gain_db);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("set_lna_gain failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    return *this;
  }

  auto set_vga_gain(uint32_t gain_db) -> HackRFDevice & {
    int result = hackrf_set_vga_gain(device_, gain_db);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("set_vga_gain failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    return *this;
  }

  auto set_amp_enable(bool enable) -> HackRFDevice & {
    int result = hackrf_set_amp_enable(device_, enable ? 1 : 0);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("set_amp_enable failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    return *this;
  }

  auto start_rx(hackrf_sample_block_cb_fn callback, void *ctx)
      -> HackRFDevice & {
    int result = hackrf_start_rx(device_, callback, ctx);
    if (result != HACKRF_SUCCESS) {
      throw std::runtime_error(std::string("start_rx failed: ") +
                               hackrf_error_name((hackrf_error)result));
    }
    streaming_ = true;
    return *this;
  }

  auto stop_rx() -> HackRFDevice & {
    if (streaming_) {
      hackrf_stop_rx(device_);
      streaming_ = false;
    }
    return *this;
  }

  auto is_streaming() const -> bool {
    return device_ && hackrf_is_streaming(device_) == HACKRF_TRUE;
  }
};
#endif // HACKRF_DEVICE_HPP