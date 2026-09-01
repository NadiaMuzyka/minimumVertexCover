#!/usr/bin/env python3
"""Plotta i risultati dell'esperimento finale di qualita' (results/exp_quality.csv):
il rapporto cardinalita'/OPT di 2-approssimato e greedy rispetto all'ILP
(verita' di riferimento), su tutto il range n=10..200, sparso e denso
separati -- media e fascia min-max sulle ripetizioni. Include anche un
confronto dei tempi di esecuzione dei tre metodi."""

import argparse
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent

ALGO_COLORS = {"approx": "tab:orange", "greedy": "tab:green", "ILP": "tab:blue"}
ALGO_LABELS = {"approx": "2-Approx", "greedy": "Greedy", "ILP": "ILP (Gurobi)"}


def parse_density(grafo):
    m = re.match(r"n\d+_(sparse|dense)_s\d+", grafo)
    return m.group(1) if m else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", default=str(ROOT / "results" / "exp_quality.csv"))
    parser.add_argument("-o", "--output-dir", default=str(ROOT / "results"))
    args = parser.parse_args()

    df_all = pd.read_csv(args.csv)
    df_all["Densita"] = df_all["Grafo"].apply(parse_density)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    opt = df_all[df_all["Algoritmo"] == "ILP"].set_index("Grafo")["Cardinalita"].to_dict()

    df = df_all[df_all["Algoritmo"].isin(["approx", "greedy"])].copy()
    df["OPT"] = df["Grafo"].map(opt)
    df = df.dropna(subset=["OPT"])
    df["Ratio"] = df["Cardinalita"] / df["OPT"]

    for density in sorted(df["Densita"].dropna().unique()):
        subset = df[df["Densita"] == density]
        fig, ax = plt.subplots(figsize=(9, 6))
        for algo, group in subset.groupby("Algoritmo"):
            stats = group.groupby("N_Nodi")["Ratio"].agg(["mean", "min", "max"]).sort_index()
            color = ALGO_COLORS.get(algo, "gray")
            ax.plot(stats.index, stats["mean"], marker="o", color=color,
                     label=ALGO_LABELS.get(algo, algo))
            ax.fill_between(stats.index, stats["min"], stats["max"], color=color, alpha=0.15)

        ax.axhline(1.0, color="black", linestyle=":", linewidth=1.8, zorder=5, label="Ottimo")
        ax.axhline(2.0, color="red", linestyle="--", linewidth=1.5, zorder=5, label="Bound teorico 2-approx")
        ax.set_xlabel("n (numero di nodi)")
        ax.set_ylabel("Cardinalita' / OPT")
        ax.set_title(f"Qualita': rapporto rispetto all'ottimo -- grafi {density}")
        ax.legend()
        ax.grid(True, linestyle="--", alpha=0.4)
        fig.tight_layout()
        out_path = out_dir / f"exp_quality_ratio_{density}.png"
        fig.savefig(out_path, dpi=150)
        print(f"Salvato: {out_path}")

    for density in sorted(df_all["Densita"].dropna().unique()):
        subset = df_all[df_all["Densita"] == density]
        fig, ax = plt.subplots(figsize=(9, 6))
        for algo, group in subset.groupby("Algoritmo"):
            stats = group.groupby("N_Nodi")["CPU_Time_sec"].mean().sort_index()
            ax.plot(stats.index, stats.values, marker="o", color=ALGO_COLORS.get(algo, "gray"),
                    label=ALGO_LABELS.get(algo, algo))
        ax.set_yscale("log")
        ax.set_xlabel("n (numero di nodi)")
        ax.set_ylabel("Tempo (s, scala log)")
        ax.set_title(f"Tempo di esecuzione -- grafi {density}")
        ax.legend()
        ax.grid(True, which="both", linestyle="--", alpha=0.4)
        fig.tight_layout()
        out_path = out_dir / f"exp_quality_tempo_{density}.png"
        fig.savefig(out_path, dpi=150)
        print(f"Salvato: {out_path}")

    # --- Grafico cardinalita' assoluta: tutti e tre gli algoritmi, ILP incluso ---
    for density in sorted(df_all["Densita"].dropna().unique()):
        subset = df_all[df_all["Densita"] == density]
        fig, ax = plt.subplots(figsize=(9, 6))
        for algo, group in subset.groupby("Algoritmo"):
            stats = group.groupby("N_Nodi")["Cardinalita"].agg(["mean", "min", "max"]).sort_index()
            color = ALGO_COLORS.get(algo, "gray")
            ax.plot(stats.index, stats["mean"], marker="o", color=color,
                     label=ALGO_LABELS.get(algo, algo))
            ax.fill_between(stats.index, stats["min"], stats["max"], color=color, alpha=0.15)

        ax.set_xlabel("n (numero di nodi)")
        ax.set_ylabel("Cardinalita' del cover trovato")
        ax.set_title(f"Cardinalita' assoluta -- grafi {density}")
        ax.legend()
        ax.grid(True, linestyle="--", alpha=0.4)
        fig.tight_layout()
        out_path = out_dir / f"exp_quality_cardinalita_{density}.png"
        fig.savefig(out_path, dpi=150)
        print(f"Salvato: {out_path}")

    print("\nRiepilogo (media del rapporto su tutte le n e ripetizioni):")
    for (algo, density), group in df.groupby(["Algoritmo", "Densita"]):
        print(f"  {ALGO_LABELS.get(algo, algo):<10} {density:<8}: media={group['Ratio'].mean():.3f}  "
              f"max={group['Ratio'].max():.3f}")


if __name__ == "__main__":
    main()