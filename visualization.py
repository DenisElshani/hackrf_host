import matplotlib.pyplot as plt
from matplotlib.widgets import RangeSlider

def plot_interactive(file_path, limit=200000):
    values = []
    marker_positions = []
    last_was_empty = False
    
    # 1. Data Parsing
    with open(file_path, 'r') as f:
        for line in f:
            if ';' not in line: continue
            tokens = line.strip().split(';')
            for token in tokens:
                clean_token = token.strip()
                if clean_token:
                    try:
                        values.append(float(clean_token))
                        last_was_empty = False
                        if len(values) >= limit: break
                    except ValueError: continue
                else:
                    if not last_was_empty and len(values) > 0:
                        marker_positions.append(len(values))
                        last_was_empty = True
            if len(values) >= limit: break

    # 2. Setup Plot
    fig, ax = plt.subplots(figsize=(15, 8))
    plt.subplots_adjust(bottom=0.25) # Make room for the slider
    
    line, = ax.plot(values, color='#1f77b4', linewidth=0.6, label='Signal')
    
    # Draw vertical markers
    for pos in marker_positions:
        ax.axvline(x=pos, color='red', linestyle='--', alpha=0.5, linewidth=1)

    ax.set_title(f"HackRF Signal - Interactive View ({len(values)} samples)")
    ax.set_xlabel("Sample Index")
    ax.set_ylabel("Value")
    ax.grid(True, linestyle=':', alpha=0.4)

    # 3. Create Range Slider (The "Scrollbar")
    ax_slider = plt.axes([0.15, 0.1, 0.7, 0.03])
    slider = RangeSlider(ax_slider, 'View Range', 0, len(values), valinit=(0, min(10000, len(values))))

    # Update function for the slider
    def update(val):
        ax.set_xlim(val[0], val[1])
        fig.canvas.draw_idle()

    slider.on_changed(update)

    # Set initial zoom (first 10k samples)
    ax.set_xlim(0, 10000)
    
    print(f"Loaded {len(values)} samples. Use the slider at the bottom to scroll/zoom.")
    plt.show()

if __name__ == "__main__":
    plot_interactive('output2_samples.txt', limit=200000)