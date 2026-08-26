#!/usr/bin/env python3
"""Confronto qualitativo Greedy vs 2-Approximation: legge results/quality.csv
(scritto da src/main_quality.cpp) e produce un bar chart per dataset con la
cardinalita' del cover trovato da ciascun algoritmo (e l'ottimo, se noto)."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", nargs="?", default=str(ROOT / "results" / "quality.csv"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "quality.png"))
    args = parser.parse_args()

    df = pd.read_csv(args.csv_file)
    datasets = df["Dataset"].unique()

    fig, ax = plt.subplots(figsize=(max(8, len(datasets) * 0.9), 6))
    width = 0.35
    x = range(len(datasets))

    approx_vals = [df[(df.Dataset == d) & (df.Algoritmo == "Approx")]["Cardinalita"].iloc[0] for d in datasets]
    greedy_vals = [df[(df.Dataset == d) & (df.Algoritmo == "Greedy")]["Cardinalita"].iloc[0] for d in datasets]

    ax.bar([i - width / 2 for i in x], approx_vals, width, label="2-Approx (matching)")
    ax.bar([i + width / 2 for i in x], greedy_vals, width, label="Greedy (grado max)")

    for i, d in enumerate(datasets):
        rows = df[df.Dataset == d]
        optimal = rows["Cardinalita_Ottima_se_nota"].dropna()
        if not optimal.empty:
            opt = optimal.iloc[0]
            ax.hlines(opt, i - width, i + width, colors="black", linestyles="dashed", linewidth=1)

    ax.set_xticks(list(x))
    ax.set_xticklabels(datasets, rotation=45, ha="right")
    ax.set_ylabel("Cardinalita' del vertex cover")
    ax.set_title("Greedy vs 2-Approximation (tratteggio nero = ottimo, dove noto)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Salvato: {args.output}")


if __name__ == "__main__":
    main()
