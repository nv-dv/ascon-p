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
        "encrypt": {"mean": 12345, "cycle/bit": 67},
        "decrypt": {"mean": 6789, "cycle/bit": 45}
    }
    """
    stats = {"encrypt": {}, "decrypt": {}}
    # Split by encryption and decryption sections
    sections = re.split(r'Encryption statistics:|Decryption statistics:', output)
    if len(sections) >= 2:
        # sections[1] = encrypt part, sections[2] = decrypt part (may include other text)
        encrypt_text = sections[1]
        decrypt_text = sections[2] if len(sections) > 2 else sections[1]
        for label, text in zip(["encrypt", "decrypt"], [encrypt_text, decrypt_text]):
            # Find all lines like "mean: 12345" or "cycle/bit: 67"
            matches = re.findall(r'([a-zA-Z/]+):\s*([\d.]+)', text)
            for key, value in matches:
                stats[label][key] = float(value) if '.' in value else int(value)

    return stats


def run_benchmark(N: int, count: int, arch: string = 'x64'):
    """
    Runs taniask benchmark and returns cycles/bit
    N: Security parameter
    count: number of iterations to average over
    arch: ISA to benchmark - x64, x86, armv7, aarch64
    """
    print(f"Running benchmark for N={N}, count={count}, arch={arch}")

    conn = fabric.Connection('linuxvm')
    with conn.cd('~/project/ascon-p'):
        result = conn.run(f'./bin_{arch}/main_{arch} {N} {count}', hide=True)
        stats = parse_benchmark_output(result.stdout)

    return stats


def plot_benchmarks(N_values, stats):
    """
    N_values: list of matrix sizes
    stats: dict of architecture -> list of dicts each like:
        {
        "encrypt": {"mean": ..., "cycle/bit": ...},
        "decrypt": {...}
        }
    """

    ideal_curve = np.log2(N_values) / (N_values ** 2)

    fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    # consistent colors
    colors = itertools.cycle(plt.rcParams["axes.prop_cycle"].by_key()["color"])

    for arch, results in stats.items():
        color = next(colors)

        cycles_enc = [m["encrypt"]["mean"] for m in results]
        cycles_dec = [m["decrypt"]["mean"] for m in results]
        cb_enc = [m["encrypt"]["cycle/bit"] for m in results]
        cb_dec = [m["decrypt"]["cycle/bit"] for m in results]

        # Mean cycle plot 
        axs[0].plot(N_values, cycles_enc, marker="o", color=color,
                    label=f"{arch} encrypt")
        axs[0].plot(N_values, cycles_dec, marker="s", linestyle="--", color=color,
                    label=f"{arch} decrypt")

        # Cycle/bit plot 
        axs[1].plot(N_values, cb_enc, marker="o", color=color,
                    label=f"{arch} encrypt")
        axs[1].plot(N_values, cb_dec, marker="s", linestyle="--", color=color,
                    label=f"{arch} decrypt")

    # TODO: Add in AES plot

    # format top
    axs[0].set_ylabel("Mean cycles")
    axs[0].set_xscale("log", base=2)
    axs[0].set_yscale("log")
    axs[0].set_title("Mean cycles per N")
    axs[0].grid(True, which="both", linestyle="--", alpha=0.5)
    axs[0].legend()

    # format bottom
    axs[1].set_ylabel("Cycles per bit")
    axs[1].set_xlabel("Matrix size N")
    axs[1].set_xscale("log", base=2)
    axs[1].set_yscale("log")
    axs[1].set_title("Cycles per bit vs N")
    axs[1].grid(True, which="both", linestyle="--", alpha=0.5)

    # scaled theory curve
    all_cb = []
    for results in stats.values():
        all_cb.extend(m["encrypt"]["cycle/bit"] for m in results)
        all_cb.extend(m["decrypt"]["cycle/bit"] for m in results)
    scale = max(all_cb) / max(ideal_curve)
    axs[1].plot(N_values, ideal_curve * scale, "--", color="black",
                label="log2(N)/N^2 (scaled)")

    axs[1].legend()

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
        help="Architecture to benchmark, x64, x86, armv7, aarch64"
    )

    args = parser.parse_args()

    N_values = np.logspace(np.log2(args.Nmin),
                           np.log2(args.Nmax),
                           num=args.Npoints,
                           base=2.0,
                           dtype=int)

    stats = {}

    for arch in args.arch:
        stats[arch] = []
        manager = enlighten.get_manager()
        pbar = manager.counter(total=len(N_values), desc='Benchmarking')

        for n in pbar(N_values):
            stats[arch].append(run_benchmark(n, args.count, arch=arch))

    plot_benchmarks(N_values, stats)


if __name__ == "__main__":
    main()

