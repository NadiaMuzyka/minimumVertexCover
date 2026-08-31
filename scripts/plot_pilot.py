#!/usr/bin/env python3
"""Plotta i risultati del pilot di validazione (results/pilot.csv, scritto da
./mvc e da scripts/exact_ilp.py): un grafico con la cardinalita' trovata da
ciascun algoritmo per ogni grafo, e due con il tempo di esecuzione (scala
log, uno per i grafi sparsi e uno per i densi)."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent

MARKERS = {"brute": "^", "bb": "o", "ILP": "s", "approx": "x", "greedy": "d"}
LABELS = {
    "brute": "Forza bruta",
    "bb": "Branch-and-Bound",
    "ILP": "ILP (Gurobi)",
    "approx": "2-Approx",
    "greedy": "Greedy",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", default=str(ROOT / "results" / "pilot.csv"))
    parser.add_argument("-o", "--output-dir", default=str(ROOT / "results"))
    args = parser.parse_args()

    df = pd.read_csv(args.csv)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    graph_order = list(dict.fromkeys(df["Grafo"]))
    df["Grafo"] = pd.Categorical(df["Grafo"], categories=graph_order, ordered=True)

    def plot(column, ylabel, title, filename, logy=False):
        fig, ax = plt.subplots(figsize=(10, 6))
        for algo, group in df.groupby("Algoritmo"):
            group = group.sort_values("Grafo")
            ax.plot(group["Grafo"], group[column],
                    marker=MARKERS.get(algo, "o"), label=LABELS.get(algo, algo))
        if logy:
            ax.set_yscale("log")
        ax.set_xlabel("Grafo")
        ax.set_ylabel(ylabel)
        ax.set_title(title)
        ax.legend()
        ax.grid(True, which="both", linestyle="--", alpha=0.4)
        plt.xticks(rotation=45, ha="right")
        fig.tight_layout()
        out_path = out_dir / filename
        fig.savefig(out_path, dpi=150)
        print(f"Salvato: {out_path}")

    plot("Cardinalita", "Cardinalita' del cover trovato",
         "Pilot: cardinalita' per algoritmo", "pilot_cardinalita.png")

    def plot_time_by_density(tag, title_suffix, filename):
        subset = df[df["Grafo"].astype(str).str.contains(tag, case=False, na=False)]
        fig, ax = plt.subplots(figsize=(8, 6))
        for algo, group in subset.groupby("Algoritmo"):
            group = group.sort_values("N_Nodi")
            ax.plot(group["N_Nodi"], group["CPU_Time_sec"],
                    marker=MARKERS.get(algo, "o"), label=LABELS.get(algo, algo))
        ax.set_yscale("log")
        ax.set_xlabel("n (numero di nodi)")
        ax.set_ylabel("Tempo di esecuzione (s, scala log)")
        ax.set_title(f"Pilot: tempo di esecuzione - grafi {title_suffix}")
        ax.legend()
        ax.grid(True, which="both", linestyle="--", alpha=0.4)
        fig.tight_layout()
        out_path = out_dir / filename
        fig.savefig(out_path, dpi=150)
        print(f"Salvato: {out_path}")

    plot_time_by_density("sparse", "sparsi", "pilot_tempo_sparso.png")
    plot_time_by_density("dense", "densi", "pilot_tempo_denso.png")


if __name__ == "__main__":
    main()