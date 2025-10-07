import fabric
from matplotlib import pyplot as plt
import argparse
import numpy as np
import re
import string
import enlighten
import itertools


def parse_benchmark_output(output: str) -> dict:
    """
    Parses the output from the taniask benchmark and returns structured metrics.

    Returns a dict like:
    {
        "taniask": {"mean": 12345, "cycle/bit": 67},
        "aes128": {"mean": 6789, "cycle/bit": 45}
    }
    """
    stats = {"taniask": {}, "aes128": {}}
    # Extract TANIASK encryption statistics
    taniask_match = re.search(
        r'Encryption statistics:\s*((?:\s*[a-zA-Z/]+:\s*[\d.]+\s*)+)',
        output, re.MULTILINE
    )
    if taniask_match:
        text = taniask_match.group(1)
        matches = re.findall(r'([a-zA-Z/]+):\s*([\d.]+)', text)
        for key, value in matches:
            stats["taniask"][key] = float(value) if '.' in value else int(value)
    # Extract TANIASK decryption statistics if needed (not specified in new format)
    # Extract AES-128 CTR encryption statistics
    aes_match = re.search(
        r'AES-128 CTR encryption statistics:\s*((?:\s*[a-zA-Z/]+:\s*[\d.]+\s*)+)',
        output, re.MULTILINE
    )
    if aes_match:
        text = aes_match.group(1)
        matches = re.findall(r'([a-zA-Z/]+):\s*([\d.]+)', text)
        for key, value in matches:
            stats["aes128"][key] = float(value) if '.' in value else int(value)

    return stats


def run_benchmark(N: int, count: int, arch: string = 'x64', simd_variant: str = None):
    """
    Runs taniask benchmark and returns cycles/bit
    N: Security parameter
    count: number of iterations to average over
    arch: ISA to benchmark - x64, x86, armv7, aarch64
    simd_variant: SIMD variant to benchmark
    """
    print(f"Running benchmark for N={N}, count={count}, arch={arch}, simd_variant={simd_variant}")

    conn = fabric.Connection('linuxvm')
    with conn.cd('~/project/ascon-p'):
        if simd_variant:
            result = conn.run(f'./bin_{arch}/main_{simd_variant} {N} {count}', hide=True)
        else:
            result = conn.run(f'./bin_{arch}/main_{arch} {N} {count}', hide=True)
        stats = parse_benchmark_output(result.stdout)

    return stats


def plot_benchmarks(N_values, stats):
    """
    N_values: list of matrix sizes
    stats: dict of architecture -> dict of simd_variant -> list of dicts each like:
        {
        "taniask": {"mean": ..., "cycle/bit": ...},
        "aes128": {"mean": ..., "cycle/bit": ...}
        }
    """

    ideal_curve = np.log2(N_values) / (N_values ** 2)

    fig, axs = plt.subplots(2, 1, figsize=(12, 9), sharex=True)

    # consistent colors
    colors = itertools.cycle(plt.rcParams["axes.prop_cycle"].by_key()["color"])
    linestyles = ['-', '--', '-.', ':']
    markers = ['o', 'x', 's', '^', 'd', 'v', '*', 'P', 'H']

    # To assign distinct styles for each (arch, simd_variant)
    style_cycle = itertools.cycle([(ls, mk) for ls in linestyles for mk in markers])

    for arch, simd_dict in stats.items():
        for simd_variant, results in simd_dict.items():
            color = next(colors)
            linestyle, marker = next(style_cycle)
            label_prefix = f"{arch} ({simd_variant})"

            # Extract TANIASK encrypt mean and cycle/bit
            taniask_means = [m["taniask"].get("mean", np.nan) for m in results]
            taniask_cpb = [m["taniask"].get("cycle/bit", np.nan) for m in results]

            # Extract AES-128 CTR cycle/bit and mean
            aes_means = [m["aes128"].get("mean", np.nan) for m in results]
            aes_cpb = [m["aes128"].get("cycle/bit", np.nan) for m in results]

            # Plot TANIASK encrypt mean cycles
            axs[0].plot(N_values, taniask_means, marker=marker, linestyle=linestyle, color=color,
                        label=f"{label_prefix} TANIASK encrypt")
            # Plot AES-128 CTR mean cycles with distinct linestyle and color
            axs[0].plot(N_values, aes_means, marker=marker, linestyle=linestyle, color=color,
                        alpha=0.6, label=f"{label_prefix} AES-128 CTR")

            # Plot TANIASK encrypt cycles per bit
            axs[1].plot(N_values, taniask_cpb, marker=marker, linestyle=linestyle, color=color,
                        label=f"{label_prefix} TANIASK encrypt")
            # Plot AES-128 CTR cycles per bit with distinct linestyle and color
            axs[1].plot(N_values, aes_cpb, marker=marker, linestyle=linestyle, color=color,
                        alpha=0.6, label=f"{label_prefix} AES-128 CTR")

    # format top
    axs[0].set_ylabel("Mean cycles")
    axs[0].set_xscale("log", base=2)
    axs[0].set_yscale("log")
    axs[0].set_title("Mean cycles")
    axs[0].grid(True, which="both", linestyle="--", alpha=0.5)
    axs[0].legend(fontsize='small', loc='best')

    # format bottom
    axs[1].set_ylabel("Cycles per bit vs N")
    axs[1].set_xlabel("Matrix size N")
    axs[1].set_xscale("log", base=2)
    axs[1].set_yscale("log")
    axs[1].set_title("Cycles per bit vs N")
    axs[1].grid(True, which="both", linestyle="--", alpha=0.5)

    # scaled theory curve
    all_cb = []
    for simd_dict in stats.values():
        for results in simd_dict.values():
            all_cb.extend(m["taniask"].get("cycle/bit", np.nan) for m in results)
            all_cb.extend(m["aes128"].get("cycle/bit", np.nan) for m in results)
    # Remove nan values for scaling
    all_cb = [v for v in all_cb if not np.isnan(v)]
    if all_cb:
        scale = max(all_cb) / max(ideal_curve)
        axs[1].plot(N_values, ideal_curve * scale, "--", color="black",
                    label="log2(N)/N^2 (scaled)")
        axs[1].legend(fontsize='small', loc='best')

    plt.tight_layout()
    plt.show()


def main():

    parser = argparse.ArgumentParser(description="Run taniask benchmarks over a range of N values.")
    # Accept multiple N values
    parser.add_argument(
        "-nmin", "--Nmin",
        type=int,
        default=8,
        help="Minimum security parameter, eg: 4, 8, 16. (default: 8)"
    )

    parser.add_argument(
        "-nmax", "--Nmax",
        type=int,
        default=512,
        help="Maximum security parameter, eg: 64, 128, 256. (default: 512)"
    )

    parser.add_argument(
        "-n", "--Npoints",
        type=int,
        default=5,
        help="Number of N's to benchmark. (default: 5)"
    )

    parser.add_argument(
        "--count",
        type=int,
        default=1000,
        help="Number of iterations per benchmark (default: 1_000)"
    )

    parser.add_argument(
        "--arch",
        type=str,
        nargs='+',
        help="Architecture to benchmark, x64, x86, aarch64, armv7"
    )

    parser.add_argument(
        "--simd",
        type=str,
        nargs='+',
        default=None,
        help="SIMD variants to benchmark. If not specified, defaults per architecture are used."
    )

    args = parser.parse_args()

    # Define default SIMD variants per architecture
    default_simd = {
        'x86': ['sse2', 'sse4.1', 'avx', 'avx2'],
        'x64': ['sse2', 'sse4.1', 'avx', 'avx2'],
        'armv7': ['neon'],
        'aarch64': ['neon']
    }

    N_values = np.logspace(np.log2(args.Nmin),
                           np.log2(args.Nmax),
                           num=args.Npoints,
                           base=2.0,
                           dtype=int)

    stats = {}

    manager = enlighten.get_manager()

    for arch in args.arch:
        # Determine SIMD variants to run
        if args.simd is not None:
            simd_variants = args.simd
        else:
            simd_variants = default_simd.get(arch, [arch])

        stats[arch] = {}
        for simd_variant in simd_variants:
            stats[arch][simd_variant] = []
            pbar = manager.counter(total=len(N_values), desc=f'Benchmarking {arch} {simd_variant}')
            for n in pbar(N_values):
                stats[arch][simd_variant].append(run_benchmark(n, args.count, arch=arch, simd_variant=simd_variant))

    plot_benchmarks(N_values, stats)


if __name__ == "__main__":
    main()
