#!/usr/bin/env python3
"""Plotta i risultati dell'Esperimento 3 (results/exp3_approx_growth.csv):
andamento del tempo del 2-approssimato al crescere di n, sparso e denso.
Due grafici: tempo vs n in scala log-log (la pendenza stima l'esponente
polinomiale), e rapporto tempo(2n)/tempo(n) vs n con le linee di
riferimento O(n), O(n^2), O(n^3)."""

import argparse
import re
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent

COLORS = {"sparse": "tab:blue", "dense": "tab:red"}


def parse_density(grafo):
    m = re.match(r"n\d+_(sparse|dense)_s\d+", grafo)
    return m.group(1) if m else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", default=str(ROOT / "results" / "exp3_approx_growth.csv"))
    parser.add_argument("-o", "--output-dir", default=str(ROOT / "results"))
    args = parser.parse_args()

    df = pd.read_csv(args.csv)
    df = df[df["Algoritmo"] == "approx"].copy()
    df["Densita"] = df["Grafo"].apply(parse_density)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    means = df.groupby(["Densita", "N_Nodi"])["CPU_Time_sec"].mean().reset_index()

    fig, ax = plt.subplots(figsize=(8, 6))
    print("Stima della pendenza (esponente polinomiale) per regressione lineare su log-log:")
    for density, group in means.groupby("Densita"):
        group = group.sort_values("N_Nodi")
        color = COLORS.get(density, "gray")
        ax.plot(group["N_Nodi"], group["CPU_Time_sec"], marker="o", color=color, label=density)

        if len(group) >= 2:
            logn = np.log(group["N_Nodi"])
            logt = np.log(group["CPU_Time_sec"])
            slope, intercept = np.polyfit(logn, logt, 1)
            print(f"  {density}: pendenza ~ {slope:.2f}  (O(n^{slope:.2f}))")

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("n (scala log)")
    ax.set_ylabel("Tempo 2-approx (s, scala log)")
    ax.set_title("Esperimento 3: tempo vs n (log-log)")
    ax.legend()
    ax.grid(True, which="both", linestyle="--", alpha=0.4)
    fig.tight_layout()
    out_path = out_dir / "exp3_approx_time.png"
    fig.savefig(out_path, dpi=150)
    print(f"Salvato: {out_path}")

    fig, ax = plt.subplots(figsize=(8, 6))
    for density, group in means.groupby("Densita"):
        group = group.sort_values("N_Nodi").reset_index(drop=True)
        ns, ratios = [], []
        for i in range(1, len(group)):
            ns.append(group.loc[i, "N_Nodi"])
            ratios.append(group.loc[i, "CPU_Time_sec"] / group.loc[i - 1, "CPU_Time_sec"])
        color = COLORS.get(density, "gray")
        ax.plot(ns, ratios, marker="o", color=color, label=density)

    for label, y in [("O(n)", 2), ("O(n^2)", 4), ("O(n^3)", 8)]:
        ax.axhline(y, linestyle="--", color="gray", alpha=0.5)
        ax.text(0.01, y, label, transform=ax.get_yaxis_transform(),
                fontsize=8, color="gray", va="bottom")

    ax.set_xscale("log")
    ax.set_xlabel("n (scala log)")
    ax.set_ylabel("Rapporto tempo(2n) / tempo(n)")
    ax.set_title("Esperimento 3: rapporto di raddoppio vs n")
    ax.legend()
    ax.grid(True, which="both", linestyle="--", alpha=0.4)
    fig.tight_layout()
    out_path2 = out_dir / "exp3_approx_ratio.png"
    fig.savefig(out_path2, dpi=150)
    print(f"Salvato: {out_path2}")


if __name__ == "__main__":
    main()