import matplotlib.pyplot as plt

# Hardcoded data taken directly from table.tex
FROBENIUS = {
    "einstein.png": [
        (80, 363.79527),
        (40, 1168.53798),
        (30, 1572.21786),
        (20, 2129.10169),
        (10, 3250.57103),
    ],
    "globe.png": [
        (20, 3259.34411),
        (10, 4934.48761),
    ],
    "greyscale.png": [
        (20, 1012.42629),
        (10, 2512.75645),
    ],
    "test.png": [
        (20, 1004.19470),
        (10, 1546.89140),
    ],
}

PER_PIXEL = {
    "einstein.png": [
        (80, 0.01075),
        (40, 0.03452),
        (30, 0.04644),
        (20, 0.06289),
        (10, 0.09602),
    ],
    "globe.png": [
        (20, 0.03460),
        (10, 0.05238),
    ],
    "greyscale.png": [
        (20, 0.00386),
        (10, 0.00959),
    ],
    "test.png": [
        (20, 0.12099),
        (10, 0.18637),
    ],
}


def plot_dataset(dataset, xlabel, ylabel, out_file):
    plt.figure(figsize=(8, 5))
    for img, points in dataset.items():
        ks = [p[0] for p in points]
        vals = [p[1] for p in points]
        plt.plot(ks, vals, marker="o", label=img)

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_file, dpi=200)
    plt.close()


plot_dataset(
    FROBENIUS, "k", "Frobenius norm (||A - A_c||)", "../figs/frobenius_error_plot.png"
)
plot_dataset(PER_PIXEL, "k", "Error per pixel", "../figs/frobenius_error_plot_pp.png")
