#!/usr/bin/env python3
"""Muro esponenziale del Branch-and-Bound: legge results/nphard.csv (scritto
da src/main_nphard.cpp) e produce due grafici semilog-y affiancati: tempo di
esecuzione vs n, e nodi esplorati vs n (con 2^n come riferimento teorico del
caso peggiore, per mostrare l'effetto della potatura)."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", nargs="?", default=str(ROOT / "results" / "nphard.csv"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "nphard.png"))
    args = parser.parse_args()

    df = pd.read_csv(args.csv_file)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 6))

    for topologia, group in df.groupby("Topologia"):
        ok = group[group.Status == "OK"].sort_values("N_Nodi")
        ax1.plot(ok["N_Nodi"], ok["CPU_Time_sec"], marker="o", label=topologia)
        ax2.plot(ok["N_Nodi"], ok["NodiEsplorati"], marker="o", label=topologia)

    n_range = sorted(df["N_Nodi"].unique())
    ax2.plot(n_range, [2 ** n for n in n_range], linestyle="--", color="gray", label="2^n (caso peggiore)")

    ax1.set_yscale("log")
    ax1.set_xlabel("N (numero di nodi)")
    ax1.set_ylabel("Tempo di esecuzione (s)")
    ax1.set_title("Tempo del Branch-and-Bound vs n")
    ax1.legend()
    ax1.grid(True, which="both", linestyle="--", alpha=0.4)

    ax2.set_yscale("log")
    ax2.set_xlabel("N (numero di nodi)")
    ax2.set_ylabel("Nodi esplorati nell'albero di ricerca")
    ax2.set_title("Effetto della potatura: nodi esplorati vs 2^n")
    ax2.legend()
    ax2.grid(True, which="both", linestyle="--", alpha=0.4)

    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Salvato: {args.output}")


if __name__ == "__main__":
    main()
