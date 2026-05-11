import uproot
import numpy as np
import matplotlib.pyplot as plt
import scipy.ndimage as ndimage
import math
import os
from slice_viewer import SliceViewer

# ==========================================
# CONFIGURATION
# ==========================================
BIN_SIZE       = 0.5    # Size of zenith angle groups in degrees
ZENITH_MAX     = 60.0   # From generator.cc
SIM_PERCENTILE = 1.0   # 0.0 to 1.0 (50% of muons used)
VOXEL_RES      = 120
BOX_SIZE       = 80.0
SIGMA_SMOOTH   = 1.0
VIEW = True

# ==========================================
# LOAD DATA AND ZENITH BINNING
# ==========================================
def load_and_process_data(folder_path="."):
    all_px, all_py, all_pz, all_angles = [], [], [], []
    all_in = []

    print(f"Scanning directory: {folder_path}...")
    
    for i in range(120):
        file_path = os.path.join(folder_path, f"output{i}.root")
        if not os.path.exists(file_path): continue
        try:
            with uproot.open(file_path) as f:
                d = f["Muons"].arrays([
                    "poca_x", "poca_y", "poca_z", "angle",
                    "in_dx", "in_dy", "in_dz"
                ], library="np")
                all_px.append(d["poca_x"])
                all_py.append(d["poca_y"])
                all_pz.append(d["poca_z"])
                all_angles.append(d["angle"])
                all_in.append(np.vstack([d["in_dx"], d["in_dy"], d["in_dz"]]).T)
        except Exception: continue

    # Combine all data
    px = np.concatenate(all_px)
    py = np.concatenate(all_py)
    pz = np.concatenate(all_pz)
    scat_angles = np.concatenate(all_angles)
    vin = np.vstack(all_in)

    # 1. Calculate Zenith Angle (Theta) relative to Y-axis
    # formula: acos(|dy| / magnitude)
    mag = np.sqrt(vin[:,0]**2 + vin[:,1]**2 + vin[:,2]**2)
    zenith_rad = np.arccos(np.abs(vin[:,1]) / mag)
    zenith_deg = np.degrees(zenith_rad)

    # 2. Define Bins
    bin_edges = np.arange(0, ZENITH_MAX + BIN_SIZE, BIN_SIZE)
    num_bins = len(bin_edges) - 1
    
    final_indices = []

    # 3. Process each bin individually
    for b in range(num_bins):
        # Find muons belonging to this zenith bin
        in_bin = np.where((zenith_deg >= bin_edges[b]) & (zenith_deg < bin_edges[b+1]))[0]
        
        if len(in_bin) == 0: continue
        
        # Calculate how many muons to keep (SIM_PERCENTILE)
        num_to_keep = math.ceil(len(in_bin) * SIM_PERCENTILE)
        
        # Randomly shuffle and pick the subset
        np.random.shuffle(in_bin)
        selected = in_bin[:num_to_keep]
        final_indices.extend(selected)

    print(f"Total Muons Found: {len(px)}")
    print(f"Muons after {SIM_PERCENTILE*100}% Percentile: {len(final_indices)}")

    idx = np.array(final_indices)
    return px[idx], py[idx], pz[idx], scat_angles[idx]

def generate_volume(x, y, z, weights):
    bins = VOXEL_RES
    ranges = [[-BOX_SIZE, BOX_SIZE]] * 3
    hist, edges = np.histogramdd((x, y, z), bins=bins, range=ranges, weights=weights)
    volume = ndimage.gaussian_filter(hist, sigma=SIGMA_SMOOTH)
    return volume, edges

if __name__ == "__main__":
    # Load filtered data
    x, y, z, w = load_and_process_data()
    
    # Reconstruct
    volume, edges = generate_volume(x, y, z, w)
    
    # Save results
    np.save("reconstructed_volume.npy", volume)
    
    if VIEW:
        viewer = SliceViewer(volume, edges)
        plt.show()
