#!/usr/bin/env python3
"""Disegna un grafo letto da un file edge-list (formato SNAP) usando networkx."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import networkx as nx

ROOT = Path(__file__).resolve().parent.parent


def read_graph(path: Path) -> nx.Graph:
    G = nx.Graph()
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            u, v = line.split()[:2]
            G.add_edge(int(u), int(v))
    return G


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("graph_file", nargs="?", default=str(ROOT / "data" / "test.txt"))
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "test_graph.png"))
    args = parser.parse_args()

    G = read_graph(Path(args.graph_file))
    print(f"Nodes: {G.number_of_nodes()}  Edges: {G.number_of_edges()}")

    pos = nx.spring_layout(G, seed=42)
    plt.figure(figsize=(6, 6))
    nx.draw(
        G, pos,
        with_labels=True,
        node_color="skyblue",
        node_size=600,
        font_size=10,
        edge_color="gray",
    )

    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=150, bbox_inches="tight")
    print(f"Saved to {out_path}")


if __name__ == "__main__":
    main()
