import matplotlib.pyplot as plt
import numpy as np

if __name__ == "__main__":
    # number of threads
    p = np.array([
        1,
        2,
        4,
        8,
        16,
        32,
        64,
        128,
    ])
    
    distuf_cap_small_sparse = np.array([
        0.241258,
        0.124208,
        0.074015,
        0.043582,
        0.024841,
        0.014806,
        0.008735,
        0.005825,
    ])
    
    distuf_cap_small = np.array([
        0.213868,
        0.104822,
        0.075437,
        0.047133,
        0.020238,
        0.016152,
        0.009397,
        0.006423,
    ])
    
    distuf_cap_small_dense = np.array([
        0.103685,
        0.05271,
        0.035353,
        0.022988,
        0.013044,
        0.012933,
        0.010032,
        0.009764,
    ])
    
    distuf_cap_med_sparse = np.array([
        7.82998,
        5.598406,
        2.358304,
        1.508014,
        0.787211,
        0.468949,
        0.260138,
        0.176624,
    ])
    
    distuf_cap_med = np.array([
        8.081038,
        3.956468,
        2.151413,
        1.63718,
        0.912338,
        0.557168,
        0.302334,
        0.180723,
    ])
    
    distuf_cap_med_dense = np.array([
        1.247273,
        0.632415,
        0.37598,
        0.211831,
        0.129001,
        0.095954,
    ])
    
    distuf_cap_large_sparse = np.array([
        22.965022,
        15.761178,
        10.433207,
        4.425186,
    ])
    
    vie_cap_small_sparse = np.array([
        0.0118552,
        0.00866039,
        0.00562393,
        0.00415957,
        0.00498321,
        0.0100668,
        0.0100668,
        0.0209579,
    ])
    
    vie_cap_small = np.array([
        0.0253361,
        0.0181725,
        0.0104914,
        0.00769472,
        0.00684469,
        0.00999797,
        0.0201698,
        0.0454983,
    ])
    
    vie_cap_small_dense = np.array([
        0.107588,
        0.0787614,
        0.0453467,
        0.0256523,
        0.0177329,
        0.0175148,
        0.0292435,
        0.0663776,
    ])
    
    vie_cap_med_sparse = np.array([
        0.251066,
        0.170951,
        0.103352,
        0.0822816,
        0.0672541,
        0.0921031,
        0.099576,
        0.159638,
    ])
    
    vie_cap_med = np.array([
        0.735558,
        0.509806,
        0.28514,
        0.201511,
        0.146608,
        0.151988,
        0.136541,
        0.164854,
    ])
    vie_cap_med_dense = np.array([
        3.497,
        2.464,
        1.36551,
        0.820794,
        0.534567,
        0.452687,
        0.302841,
        0.274112,
    ])
    
    vie_cap_large_sparse = np.array([
        3.99231,
        3.13208,
        1.87654,
        1.02763,
        1.05058,
        1.25639,
        1.22078,
        1.16646,
    ])
    
    vie_cap_large = np.array([
        10.1935,
        7.11435,
        4.28717,
        2.61366,
        1.87589,
        1.71326,
        1.56258,
        1.39997,
    ])

    fig, ax = plt.subplots(figsize=(10, 6))

    # plot data
    ax.loglog(p, vie_cap_small_sparse, 'v-', color='#1f77b4', alpha=0.25)
    ax.loglog(p, vie_cap_small, '^-', color='#ff7f0e', alpha=0.25)
    ax.loglog(p, vie_cap_small_dense, '*-', color='#2ca02c', alpha=0.25)
    ax.loglog(p, vie_cap_med_sparse, '|-', color='#d62728', alpha=0.25)
    ax.loglog(p, vie_cap_med, 's-', color='#9467bd', alpha=0.25)
    ax.loglog(p, vie_cap_med_dense, 'o-', color='#8c564b', alpha=0.25)
    ax.loglog(p, vie_cap_large_sparse, 'd-', color='#e377c2', alpha=0.25)
    ax.loglog(p, vie_cap_large, 'x-', color='#808080', alpha=0.25)
    
    ax.loglog(p, distuf_cap_small_sparse, 'v-', color='#1f77b4')
    ax.loglog(p, distuf_cap_small, '^-', color='#ff7f0e')
    ax.loglog(p, distuf_cap_small_dense, '*-', color='#2ca02c')
    ax.loglog(p, distuf_cap_med_sparse, '|-', color='#d62728')
    ax.loglog(p, distuf_cap_med, 's-', color='#9467bd')
    ax.loglog(np.array([4,8,16,32,64,128]), distuf_cap_med_dense, 'o-', color='#8c564b')
    ax.loglog(np.array([8,16,32,64]), distuf_cap_large_sparse, 'd-', color='#e377c2')

    # plot 0/-1 slope line
    x = np.logspace(np.log2(p[0]), np.log2(p[-1]), 100, base=2)
    # ax.loglog(x, 10 * x ** 0, 'k--')
    ax.loglog(x, 10 * x ** -1, 'k--')

    ax.legend([
        "OpenMP Small (n=2^15) Sparse (k=32)",
        "OpenMP Small (n=2^15) Regular (k=64)",
        "OpenMP Small (n=2^15) Dense (k=256)",
        "OpenMP Medium (n=2^20) Sparse (k=32)",
        "OpenMP Medium (n=2^20) Regular (k=64)",
        "OpenMP Medium (n=2^20) Dense (k=256)",
        "OpenMP Large (n=2^24) Sparse (k=32)",
        "OpenMP Large (n=2^24) Regular (k=64)",
        "UPC++ Small (n=2^15) Sparse (k=32)",
        "UPC++ Small (n=2^15) Regular (k=64)",
        "UPC++ Small (n=2^15) Dense (k=256)",
        "UPC++ Medium (n=2^20) Sparse (k=32)",
        "UPC++ Medium (n=2^20) Regular (k=64)",
        "UPC++ Medium (n=2^20) Dense (k=256)",
        "UPC++ Large (n=2^24) Sparse (k=32)",
        # "0 Slope", 
        "-1 Slope"
    ], bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.tight_layout(pad=2.5)
    plt.xscale('log', base=2)
    plt.xlabel("Execution units in 1 node (# threads / # processes)")
    plt.ylabel("Time (s)")
    plt.suptitle("Shared Memory (OpenMP) CAPFOREST vs. Distributed Memory (UPC++) CAPFOREST")
    plt.savefig("benchmark.png")