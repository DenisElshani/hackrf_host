import matplotlib.pyplot as plt
from collections import deque

def plot_last_samples(file_path, tail_limit=25000):
    # Deque will automatically discard old items as new ones are added
    recent_values = deque(maxlen=tail_limit)
    # We store markers relative to the total count to calculate their final position
    all_marker_indices = []
    total_samples_processed = 0
    last_token_was_value = False

    with open(file_path, 'r') as f:
        for line in f:
            if ';' not in line:
                continue
            
            # Split by semicolon to get samples and empty strings (markers)
            tokens = line.strip().split(';')
            
            for token in tokens:
                clean_token = token.strip()
                
                if clean_token:
                    try:
                        val = float(clean_token)
                        recent_values.append(val)
                        total_samples_processed += 1
                        last_token_was_value = True
                    except ValueError:
                        continue
                else:
                    # An empty token indicates a ';'
                    # In your C++ code, ";;" marks the end of a bit
                    if last_token_was_value:
                        all_marker_indices.append(total_samples_processed)
                        last_token_was_value = False

    # Calculate which markers actually fall within our "last 25000" window
    start_index = max(0, total_samples_processed - tail_limit)
    final_values = list(recent_values)
    
    # Adjust marker positions to be relative to the start of our plot (0 to 25000)
    visible_markers = [m - start_index for m in all_marker_indices if m > start_index]

    # Plotting
    plt.figure(figsize=(15, 6))
    plt.plot(final_values, color='#1f77b4', linewidth=0.8, label='Signal (Last 25k)')
    
    for i, pos in enumerate(visible_markers):
        label = "Divider (;;)" if i == 0 else ""
        plt.axvline(x=pos, color='red', linestyle='--', alpha=0.6, linewidth=1.2, label=label)

    plt.title(f"HackRF Signal - Final {len(final_values)} Samples (Tail of file)")
    plt.xlabel(f"Sample Index (Offset from start: {start_index})")
    plt.ylabel("Value")
    plt.legend(loc='upper right')
    plt.grid(True, linestyle=':', alpha=0.4)
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_last_samples('output2_samples.txt', tail_limit=25000)