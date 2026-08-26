#!/usr/bin/env python3
"""Confronto B&B vs ILP/Gurobi: unisce results/bb.csv (scritto da
src/main_bb_vs_ilp.cpp) e results/ilp.csv (scritto da scripts/exact_ilp.py,
invocato su ogni grafo generato dal primo), verifica che le due cardinalita'
coincidano per ogni grafo (sanity check di ottimalita') e plotta il tempo di
esecuzione log-y vs n per entrambi."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bb-csv", default=str(ROOT / "results" / "bb.csv"))
    parser.add_argument("--ilp-csv", default=str(ROOT / "results" / "ilp.csv"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "bb_vs_ilp.png"))
    args = parser.parse_args()

    bb = pd.read_csv(args.bb_csv)
    ilp = pd.read_csv(args.ilp_csv)

    merged = bb.merge(ilp, on="Grafo", suffixes=("_BB", "_ILP"))
    mismatches = merged[merged["Cardinalita_BB"] != merged["Cardinalita_ILP"]]
    if not mismatches.empty:
        print("ATTENZIONE: cardinalita' non coincidenti tra B&B e ILP per:")
        print(mismatches[["Grafo", "Cardinalita_BB", "Cardinalita_ILP"]].to_string(index=False))
    else:
        print(f"Verifica ottimalita' superata su {len(merged)} grafi: B&B e ILP concordano.")

    fig, ax = plt.subplots(figsize=(8, 6))
    bb_ok = bb[bb.Status == "OK"].sort_values("N_Nodi")
    ilp_ok = ilp[ilp.Status == "OPTIMAL"].sort_values("N_Nodi")
    ax.plot(bb_ok["N_Nodi"], bb_ok["CPU_Time_sec"], marker="o", label="Branch-and-Bound (custom)")
    ax.plot(ilp_ok["N_Nodi"], ilp_ok["CPU_Time_sec"], marker="s", label="ILP (Gurobi)")

    ax.set_yscale("log")
    ax.set_xlabel("N (numero di nodi)")
    ax.set_ylabel("Tempo di esecuzione (s)")
    ax.set_title("Branch-and-Bound vs ILP/Gurobi")
    ax.legend()
    ax.grid(True, which="both", linestyle="--", alpha=0.4)
    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Salvato: {args.output}")


if __name__ == "__main__":
    main()
