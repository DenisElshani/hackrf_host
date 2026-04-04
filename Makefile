# Makefile for HackRF examples
# Set up paths
HACKRF_INCLUDE := /home/elsha/UGent/ComputerSciences/tweedeMaster/Barcelona/HackerRF/hackrf/host/libhackrf/src
HACKRF_LIB := /home/elsha/UGent/ComputerSciences/tweedeMaster/Barcelona/HackerRF/hackrf/host/build/libhackrf/src
CFLAGS := -I$(HACKRF_INCLUDE) -std=c99 -Wall -Wextra -Wno-unused-parameter
CXXFLAGS := -I$(HACKRF_INCLUDE) -std=c++17 -Wall -Wextra -Wno-unused-parameter
LDFLAGS := -L$(HACKRF_LIB) -lhackrf -lusb-1.0 -lpthread

# Compilers
CC := gcc
CXX := g++

# Targets
TARGETS := hackrf_rx

# Default target
all: $(TARGETS)

# C++ example (modern C++ with STL)
hackrf_rx: hackrf_rx.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# OOK Decoder example
# ook_receiver: ook_receiver.cpp hackrf_device.hpp iq_buffer.hpp ook_decoder.hpp
# $(CXX) $(CXXFLAGS) -o $@ ook_receiver.cpp $(LDFLAGS) -lpthread
# $(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Build compile_commands.json for clangd
.PHONY: compile-commands
compile-commands:
	@echo "Generating compile_commands.json..."
	@python3 -c "import json; cmds = [ \
		{'directory': '$$(pwd)', 'command': '$(CC) $(CFLAGS) -c simple_hackrf_rx.c -o simple_hackrf_rx.o', 'file': 'simple_hackrf_rx.c'}, \
		{'directory': '$$(pwd)', 'command': '$(CC) $(CFLAGS) -lm -c hackrf_rx_processor.c -o hackrf_rx_processor.o', 'file': 'hackrf_rx_processor.c'}, \
		{'directory': '$$(pwd)', 'command': '$(CC) $(CFLAGS) -lpthread -lm -c hackrf_rx_threaded.c -o hackrf_rx_threaded.o', 'file': 'hackrf_rx_threaded.c'}, \
		{'directory': '$$(pwd)', 'command': '$(CXX) $(CXXFLAGS) -c hackrf_rx.cpp -o hackrf_rx.o', 'file': 'hackrf_rx.cpp'}, \
		{'directory': '$$(pwd)', 'command': '$(CXX) $(CXXFLAGS) -c main.cpp -o main.o', 'file': 'main.cpp'} \
	]; json.dump(cmds, open('compile_commands.json', 'w'), indent=2)"

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(TARGETS) *.o

# Help target
.PHONY: help
help:
	@echo "HackRF Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make                    - Build all examples"
	@echo "  make simple_hackrf_rx   - Build minimal example"
	@echo "  make hackrf_rx_processor - Build processor example"
	@echo "  make hackrf_rx_threaded - Build threaded example"
	@echo "  make compile-commands   - Generate compile_commands.json for clangd"
	@echo "  make clean              - Remove build artifacts"
	@echo "  make help               - Show this help message"
	@echo ""
	@echo "Environment:"
	@echo "  HACKRF_INCLUDE = $(HACKRF_INCLUDE)"
	@echo "  CC             = $(CC)"
	@echo "  CFLAGS         = $(CFLAGS)"
	@echo "  LDFLAGS        = $(LDFLAGS)"

.PHONY: check-includes
check-includes:
	@echo "Checking if hackrf.h is accessible..."
	@if [ -f "$(HACKRF_INCLUDE)/hackrf.h" ]; then \
		echo "✓ Found: $(HACKRF_INCLUDE)/hackrf.h"; \
	else \
		echo "✗ NOT FOUND: $(HACKRF_INCLUDE)/hackrf.h"; \
	fi
