#!/usr/bin/env python3
"""
analyze_iq_data.py - Analyze HackRF I/Q sample files

This script reads raw I/Q sample files from HackRF and provides analysis:
- Power spectrum (FFT)
- Time-domain plots
- Statistics (PAPR, Peak, RMS)
- Constellation diagram

Requirements:
    pip install numpy matplotlib scipy

Usage:
    ./analyze_iq_data.py data.iq <sample_rate_MHz> [--fft_size N]
    ./analyze_iq_data.py data.iq 10 --show-spectrum --show-time
"""

import numpy as np
import sys
import argparse
from pathlib import Path

def read_iq_file(filename, max_samples=None):
    """
    Read a raw I/Q sample file from HackRF
    
    File format: interleaved signed 8-bit I/Q samples
    [I0, Q0, I1, Q1, I2, Q2, ...]
    """
    try:
        # Read raw bytes
        with open(filename, 'rb') as f:
            data = np.fromfile(f, dtype=np.int8)
        
        # Limit samples if requested
        if max_samples:
            data = data[:max_samples * 2]
        
        # Reshape to I/Q pairs and convert to complex
        iq_samples = data[::2].astype(np.float32) + 1j * data[1::2].astype(np.float32)
        
        # Normalize to [-1, 1]
        iq_samples = iq_samples / 128.0
        
        return iq_samples
    
    except Exception as e:
        print(f"ERROR: Cannot read file: {e}")
        return None

def compute_statistics(iq_samples):
    """Compute signal statistics"""
    power = np.abs(iq_samples) ** 2
    
    stats = {
        'num_samples': len(iq_samples),
        'rms_level': np.sqrt(np.mean(power)),
        'peak_level': np.max(np.abs(iq_samples)),
        'peak_power_linear': np.max(power),
        'avg_power_linear': np.mean(power),
        'papr': np.max(power) / np.mean(power),  # Peak-to-Average Power Ratio
    }
    
    # Convert to dBm (assuming 50 ohm, 1V reference)
    stats['rms_dbm'] = 20 * np.log10(stats['rms_level']) - 30
    stats['peak_dbm'] = 20 * np.log10(stats['peak_level']) - 30
    stats['avg_dbm'] = 10 * np.log10(stats['avg_power_linear']) - 30
    stats['peak_power_dbm'] = 10 * np.log10(stats['peak_power_linear']) - 30
    
    return stats

def compute_spectrum(iq_samples, sample_rate_hz):
    """Compute power spectrum using FFT"""
    # Zero-pad for better frequency resolution
    N = len(iq_samples)
    fft_size = 2 ** int(np.ceil(np.log2(N)))
    
    # Compute FFT
    fft_result = np.fft.fft(iq_samples, fft_size)
    
    # Power spectrum (dB scale)
    power = np.abs(fft_result) ** 2 / fft_size
    power_db = 10 * np.log10(power + 1e-12)
    
    # Frequency axis (centered)
    freq = np.fft.fftfreq(fft_size, 1.0 / sample_rate_hz)
    
    # Shift to center at 0 Hz
    power_db = np.fft.fftshift(power_db)
    freq = np.fft.fftshift(freq)
    
    return freq, power_db, power

def print_statistics(filename, sample_rate_hz, stats):
    """Print statistics to console"""
    print(f"\n{'='*60}")
    print(f"  I/Q Data Analysis: {filename}")
    print(f"{'='*60}")
    print(f"\nAcquisition Parameters:")
    print(f"  Sample Rate: {sample_rate_hz / 1e6:.1f} MSPS")
    print(f"  Total Samples: {stats['num_samples']:,}")
    print(f"  Duration: {stats['num_samples'] / sample_rate_hz * 1e3:.2f} ms")
    
    print(f"\nSignal Statistics:")
    print(f"  RMS Level: {stats['rms_level']:.4f} (linear) = {stats['rms_dbm']:.2f} dBm")
    print(f"  Peak Level: {stats['peak_level']:.4f} (linear) = {stats['peak_dbm']:.2f} dBm")
    print(f"  Average Power: {stats['avg_power_linear']:.6f} (linear) = {stats['avg_dbm']:.2f} dBm")
    print(f"  Peak Power: {stats['peak_power_linear']:.6f} (linear) = {stats['peak_power_dbm']:.2f} dBm")
    print(f"  PAPR (Peak-to-Avg): {10 * np.log10(stats['papr']):.2f} dB ({stats['papr']:.1f}x)")
    
    # Dynamic range
    dr = stats['peak_dbm'] - (stats['avg_dbm'] - 40)  # -40dB quietness threshold
    print(f"  Dynamic Range: {dr:.1f} dB")
    
    print()

def plot_spectrum(freq, power_db, sample_rate_hz, title="Power Spectrum"):
    """Plot power spectrum"""
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("ERROR: matplotlib not installed (pip install matplotlib)")
        return
    
    fig, ax = plt.subplots(figsize=(12, 5))
    
    # Limit to displayed range
    freq_mhz = freq / 1e6
    idx = np.argsort(np.abs(freq))
    
    ax.plot(freq_mhz, power_db, linewidth=0.5)
    ax.set_xlabel("Frequency (MHz)")
    ax.set_ylabel("Power (dB)")
    ax.set_title(title)
    ax.grid(True, alpha=0.3)
    ax.set_ylim([np.max(power_db) - 80, np.max(power_db) + 5])
    
    plt.tight_layout()
    plt.show()

def plot_time_domain(iq_samples, sample_rate_hz, max_samples=10000):
    """Plot time-domain I/Q waveform"""
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("ERROR: matplotlib not installed (pip install matplotlib)")
        return
    
    # Limit to max_samples for clarity
    n = min(len(iq_samples), max_samples)
    time = np.arange(n) / sample_rate_hz
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 6))
    
    # In-phase
    ax1.plot(time * 1e6, np.real(iq_samples[:n]), label='I', linewidth=0.5)
    ax1.set_ylabel("In-phase (I)")
    ax1.set_title(f"Time Domain I/Q (first {n} samples)")
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # Quadrature
    ax2.plot(time * 1e6, np.imag(iq_samples[:n]), label='Q', color='orange', linewidth=0.5)
    ax2.set_xlabel("Time (µs)")
    ax2.set_ylabel("Quadrature (Q)")
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    plt.tight_layout()
    plt.show()

def plot_constellation(iq_samples, max_points=10000):
    """Plot I/Q constellation diagram"""
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("ERROR: matplotlib not installed (pip install matplotlib)")
        return
    
    # Subsample for clarity
    step = max(1, len(iq_samples) // max_points)
    samples = iq_samples[::step]
    
    fig, ax = plt.subplots(figsize=(8, 8))
    
    ax.scatter(np.real(samples), np.imag(samples), alpha=0.3, s=1)
    ax.set_xlabel("In-phase (I)")
    ax.set_ylabel("Quadrature (Q)")
    ax.set_title(f"Constellation Diagram ({len(samples)} points)")
    ax.grid(True, alpha=0.3)
    ax.set_aspect('equal')
    
    # Add unit circle reference
    theta = np.linspace(0, 2*np.pi, 100)
    ax.plot(np.cos(theta), np.sin(theta), 'r--', alpha=0.3, label='Unit circle')
    ax.legend()
    
    plt.tight_layout()
    plt.show()

def main():
    parser = argparse.ArgumentParser(
        description='Analyze HackRF I/Q sample files',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  ./analyze_iq_data.py data.iq 10
  ./analyze_iq_data.py data.iq 10 --show-spectrum
  ./analyze_iq_data.py data.iq 10 --show-constellation --max-samples 100000
        '''
    )
    
    parser.add_argument('filename', help='I/Q sample file (raw int8)')
    parser.add_argument('sample_rate', type=float, help='Sample rate in MHz')
    parser.add_argument('--max-samples', type=int, default=None,
                       help='Maximum samples to load')
    parser.add_argument('--show-spectrum', action='store_true',
                       help='Show FFT power spectrum')
    parser.add_argument('--show-time', action='store_true',
                       help='Show time-domain waveform')
    parser.add_argument('--show-constellation', action='store_true',
                       help='Show I/Q constellation')
    parser.add_argument('--fft-size', type=int, default=None,
                       help='FFT size (for spectrum)')
    
    args = parser.parse_args()
    
    # Check file exists
    if not Path(args.filename).exists():
        print(f"ERROR: File not found: {args.filename}")
        sys.exit(1)
    
    sample_rate_hz = int(args.sample_rate * 1e6)
    
    print(f"Loading {args.filename}...")
    iq_samples = read_iq_file(args.filename, max_samples=args.max_samples)
    
    if iq_samples is None:
        sys.exit(1)
    
    # Compute statistics
    stats = compute_statistics(iq_samples)
    print_statistics(args.filename, sample_rate_hz, stats)
    
    # Compute spectrum
    print("Computing FFT...")
    freq, power_db, power = compute_spectrum(iq_samples, sample_rate_hz)
    
    # Find peak frequency
    peak_idx = np.argmax(power_db)
    peak_freq = freq[peak_idx]
    peak_power = power_db[peak_idx]
    print(f"Peak Frequency: {peak_freq / 1e6:.3f} MHz at {peak_power:.1f} dB")
    
    # Show plots if requested
    if args.show_spectrum:
        plot_spectrum(freq, power_db, sample_rate_hz)
    
    if args.show_time:
        plot_time_domain(iq_samples, sample_rate_hz)
    
    if args.show_constellation:
        plot_constellation(iq_samples)
    
    print("\nUse --show-spectrum, --show-time, or --show-constellation for plots")

if __name__ == '__main__':
    main()
