import fabric
from matplotlib import pyplot as plt
import argparse
import numpy as np
import re
import string

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
    print(f"Running benchmark for N={N}, count={count}")

    conn = fabric.Connection('linuxvm')
    with conn.cd('~/project/ascon-p'):
        result = conn.run(f'./bin_{arch}/main_{arch} {N} {count}', hide=True)
        stats = parse_benchmark_output(result.stdout)

    return stats

def plot_benchmarks(N_values, stats_x64, stats_x86):
    """
    N_values: list or array of matrix sizes
    stats_x64, stats_x86: list of dicts returned by run_benchmark, format:
        {
            'encrypt': {'mean': ..., 'cycle/bit': ...},
            'decrypt': {'mean': ..., 'cycle/bit': ...}
        }
    """
    # Prepare data
    cycles_x64_enc = [m['encrypt']['mean'] for m in stats_x64]
    cycles_x64_dec = [m['decrypt']['mean'] for m in stats_x64]
    cb_x64_enc = [m['encrypt']['cycle/bit'] for m in stats_x64]
    cb_x64_dec = [m['decrypt']['cycle/bit'] for m in stats_x64]

    cycles_x86_enc = [m['encrypt']['mean'] for m in stats_x86]
    cycles_x86_dec = [m['decrypt']['mean'] for m in stats_x86]
    cb_x86_enc = [m['encrypt']['cycle/bit'] for m in stats_x86]
    cb_x86_dec = [m['decrypt']['cycle/bit'] for m in stats_x86]

    # Theoretical curve: log2(N)/N^2 
    N_theory = np.array(N_values)
    theory_curve = np.log2(N_theory) / (N_theory ** 2)

    # Create figure
    fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    # --- Mean cycles plot ---
    axs[0].plot(N_values, cycles_x64_enc, marker='o', color='tab:blue', label='Encrypt x64')
    axs[0].plot(N_values, cycles_x64_dec, marker='s', color='tab:cyan', label='Decrypt x64')
    axs[0].plot(N_values, cycles_x86_enc, marker='o', color='tab:red', label='Encrypt x86')
    axs[0].plot(N_values, cycles_x86_dec, marker='s', color='tab:pink', label='Decrypt x86')
    axs[0].set_ylabel("Mean cycles")
    axs[0].set_xscale('log', base=2)
    axs[0].set_yscale('log')
    axs[0].set_title("Mean cycles per N")
    axs[0].legend()
    axs[0].grid(True, which='both', linestyle='--', alpha=0.5)

    # --- Cycle/bit plot ---
    axs[1].plot(N_values, cb_x64_enc, marker='o', color='tab:blue', label='Encrypt x64')
    axs[1].plot(N_values, cb_x64_dec, marker='s', color='tab:cyan', label='Decrypt x64')
    axs[1].plot(N_values, cb_x86_enc, marker='o', color='tab:red', label='Encrypt x86')
    axs[1].plot(N_values, cb_x86_dec, marker='s', color='tab:pink', label='Decrypt x86')

    # Overlay theoretical curve (scaled for visibility)
    scale = max(cb_x64_enc + cb_x64_dec + cb_x86_enc + cb_x86_dec) / max(theory_curve)
    axs[1].plot(N_theory, theory_curve * scale, '--', color='black', label='log2(N)/N^2 (scaled)')

    axs[1].set_xlabel("Matrix size N")
    axs[1].set_ylabel("Cycles / bit")
    axs[1].set_xscale('log', base=2)
    axs[1].set_yscale('log')
    axs[1].grid(True, which='both', linestyle='--', alpha=0.5)
    axs[1].set_title("Cycle/bit per N")
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
        default='x64',
        help="Architecture to benchmark, x64, x86, armv7, aarch64 (default: x64)"
    )

    args = parser.parse_args()

    N_values = np.logspace(np.log2(args.Nmin),
                           np.log2(args.Nmax),
                           num=args.Npoints,
                           base=2.0,
                           dtype=int)

    stats_x64 = []
    stats_x86 = []
    for n in N_values:
        stats_x64.append(run_benchmark(n, args.count, arch='x64'))
        stats_x86.append(run_benchmark(n, args.count, arch='x86'))

    plot_benchmarks(N_values, stats_x64, stats_x86)
    

if __name__ == "__main__":
    main()

