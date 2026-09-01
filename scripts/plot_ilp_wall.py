#!/usr/bin/env python3
"""Plotta i risultati dell'Esperimento 2 (results/exp2_ilp_wall.csv): tempo
di esecuzione dell'ILP in funzione di n, sparso e denso, con il muro (primo
n in TIME_LIMIT) evidenziato per ciascuna densita', se esiste nel range
testato."""

import argparse
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent

COLORS = {"sparse": "tab:blue", "dense": "tab:red"}


def parse_density(grafo):
    m = re.match(r"n\d+_(sparse|dense)_s\d+", grafo)
    return m.group(1) if m else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", default=str(ROOT / "results" / "exp2_ilp_wall.csv"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "exp2_ilp_wall.png"))
    parser.add_argument("--timeout", type=float, default=120.0,
                         help="Timeout usato nell'esperimento, per disegnare la linea del tetto")
    args = parser.parse_args()

    df = pd.read_csv(args.csv)
    df = df[df["Algoritmo"] == "ILP"].copy()
    df["Densita"] = df["Grafo"].apply(parse_density)

    fig, ax = plt.subplots(figsize=(9, 6))

    for density, group in df.groupby("Densita"):
        color = COLORS.get(density, "gray")

        ok = group[group["Status"] == "OPTIMAL"]
        tl = group[group["Status"] == "TIME_LIMIT"]
        ax.scatter(ok["N_Nodi"], ok["CPU_Time_sec"], color=color, alpha=0.35, marker="o", s=25)
        ax.scatter(tl["N_Nodi"], tl["CPU_Time_sec"], color=color, alpha=0.6, marker="x", s=45)

        means = group.groupby("N_Nodi")["CPU_Time_sec"].mean().sort_index()
        ax.plot(means.index, means.values, color=color, label=f"{density} (media)", linewidth=2)

        if not tl.empty:
            wall_n = tl["N_Nodi"].min()
            ax.axvline(wall_n, color=color, linestyle="--", alpha=0.5)
            ax.annotate(f"muro {density}: n={wall_n}", xy=(wall_n, args.timeout),
                        xytext=(5, 5), textcoords="offset points", color=color, fontsize=9)
        else:
            ax.annotate(f"nessun muro {density} (fino a n={group['N_Nodi'].max()})",
                        xy=(group["N_Nodi"].max(), group["CPU_Time_sec"].max()),
                        xytext=(-5, 10), textcoords="offset points", color=color, fontsize=9,
                        ha="right")

    ax.axhline(args.timeout, color="black", linestyle=":", linewidth=1,
               label=f"timeout ({args.timeout:.0f}s)")
    ax.set_yscale("log")
    ax.set_xlabel("n (numero di nodi)")
    ax.set_ylabel("Tempo ILP (s, scala log)")
    ax.set_title("Esperimento 2: muro esponenziale dell'ILP")
    ax.legend()
    ax.grid(True, which="both", linestyle="--", alpha=0.4)
    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Salvato: {args.output}")

    print("\nRiepilogo:")
    for density, group in df.groupby("Densita"):
        tl = group[group["Status"] == "TIME_LIMIT"]
        if not tl.empty:
            print(f"  {density}: muro a n={tl['N_Nodi'].min()}")
        else:
            print(f"  {density}: nessun timeout osservato (muro oltre n={group['N_Nodi'].max()})")


if __name__ == "__main__":
    main()