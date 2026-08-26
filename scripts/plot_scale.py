#!/usr/bin/env python3
"""Scalabilita' al crescere di n: legge results/scalability.csv (scritto da
src/main_scale.cpp) e produce un plot log-log tempo vs n, una curva per
algoritmo. Le righe con Status=TIMEOUT vengono marcate ma non collegate."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", nargs="?", default=str(ROOT / "results" / "scalability.csv"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "scalability.png"))
    args = parser.parse_args()

    df = pd.read_csv(args.csv_file)

    fig, ax = plt.subplots(figsize=(8, 6))
    for algo, group in df.groupby("Algoritmo"):
        ok = group[group.Status == "OK"].sort_values("N_Nodi")
        timeout = group[group.Status == "TIMEOUT"]
        ax.plot(ok["N_Nodi"], ok["CPU_Time_sec"], marker="o", label=algo)
        if not timeout.empty:
            ax.scatter(timeout["N_Nodi"], timeout["CPU_Time_sec"], marker="x", color="red",
                       label=f"{algo} (timeout)" if algo not in ax.get_legend_handles_labels()[1] else None)

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("N (numero di nodi)")
    ax.set_ylabel("Tempo di esecuzione (s)")
    ax.set_title("Scalabilita' degli algoritmi al crescere di n")
    ax.legend()
    ax.grid(True, which="both", linestyle="--", alpha=0.4)
    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Salvato: {args.output}")


if __name__ == "__main__":
    main()
