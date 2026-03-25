#include "hackrf_device.hpp"
#include "iq_buffer.hpp"
#include "signal_process.hpp"

#include <atomic>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unistd.h>

// Signal handler
std::atomic<bool> should_exit(false); // Global atomic flag for Ctrl+C
void signal_handler(int sig) {
  should_exit = true;
  std::cerr << "\nCaught signal " << sig << ", stopping...\n";
}

// global IQ buffer
IQBuffer iq_buffer(1024);

int rx_callback(hackrf_transfer *transfer) {
  if (transfer == nullptr || transfer->buffer == nullptr) {
    std::cerr << "ERROR: Null transfer or buffer\n";
    return -1;
  }
  size_t num_samples = transfer->valid_length / 2; // 2 bytes per sample
  for (size_t i = 0; i < num_samples; i++) {
    int8_t I = transfer->buffer[2 * i];
    int8_t Q = transfer->buffer[2 * i + 1];
    iq_buffer.push(I, Q);
  }
  return 0;
}

int main(int argc, char **argv) {
  try {
    // Configuration
    uint64_t frequency = 100e6;  // 100 MHz
    uint32_t sample_rate = 10e6; // 10 MSPS
    uint32_t lna_gain = 32;      // dB
    uint32_t vga_gain = 20;      // dB
    const char *serial = nullptr;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
      if (std::strcmp(argv[i], "--serial") == 0 && i + 1 < argc) {
        serial = argv[++i];
      } else if (std::strcmp(argv[i], "--freq") == 0 && i + 1 < argc) {
        frequency = std::atof(argv[++i]) * 1e6;
      } else if (std::strcmp(argv[i], "--rate") == 0 && i + 1 < argc) {
        sample_rate = std::atof(argv[++i]) * 1e6;
      } else if (std::strcmp(argv[i], "--help") == 0) {
        std::cout << "Usage: " << argv[0] << " [options]\n"
                  << "  --serial <SN>   Device serial number\n"
                  << "  --freq <MHz>    Center frequency (default: 100)\n"
                  << "  --rate <MSPS>   Sample rate (default: 10)\n"
                  << "  --help          Show this help\n";
        return 0;
      }
    }

    // Setup signal handler
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "=== C++ HackRF Receiver ===\n"
              << "Frequency: " << frequency / 1e6 << " MHz\n"
              << "Sample Rate: " << sample_rate / 1e6 << " MSPS\n"
              << "LNA Gain: " << lna_gain << " dB\n"
              << "VGA Gain: " << vga_gain << " dB\n"
              << "Device: " << (serial ? serial : "first available") << "\n\n";

    HackRFDevice device;
    device.open(serial)
        .set_frequency(frequency)
        .set_sample_rate(sample_rate)
        .set_lna_gain(lna_gain)
        .set_vga_gain(vga_gain)
        .set_amp_enable(true)
        .start_rx(rx_callback, nullptr);
    std::cout << "Streaming started. Press Ctrl+C to stop.\n\n";

    // Main loop
    SignalProcessor processor;
    while (device.is_streaming() && !should_exit) {
      processor.process(iq_buffer);
    }

    // Cleanup (automatic via RAII)
    std::cout << "\nStopping...\n";
    device.stop_rx();

    std::cout << "Done.\n";
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
}
