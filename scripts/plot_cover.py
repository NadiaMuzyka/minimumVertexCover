#!/usr/bin/env python3
"""Disegna il grafo con il vertex cover trovato da approx evidenziato."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import networkx as nx

from plot_graph import read_graph

ROOT = Path(__file__).resolve().parent.parent


def read_cover(path: Path) -> set:
    with open(path) as f:
        f.readline()  # cardinalita' (ridondante, ricavabile da len(cover))
        ids = f.readline().split()
    return {int(x) for x in ids}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("graph_file", nargs="?", default=str(ROOT / "data" / "test.txt"))
    parser.add_argument("cover_file", nargs="?", default=str(ROOT / "results" / "approx_cover.txt"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "approx_cover.png"))
    args = parser.parse_args()

    G = read_graph(Path(args.graph_file))
    cover = read_cover(Path(args.cover_file))

    colors = ["tomato" if node in cover else "skyblue" for node in G.nodes()]

    pos = nx.spring_layout(G, seed=42)
    plt.figure(figsize=(6, 6))
    nx.draw(
        G, pos,
        with_labels=True,
        node_color=colors,
        node_size=600,
        font_size=10,
        edge_color="gray",
    )

    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=150, bbox_inches="tight")
    print(f"Vertex cover: {len(cover)} nodi -> {out_path}")


if __name__ == "__main__":
    main()
