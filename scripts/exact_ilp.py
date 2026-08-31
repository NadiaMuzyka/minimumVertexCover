#!/usr/bin/env python3
"""Risolve il Minimum Vertex Cover in modo esatto formulandolo come ILP e
risolvendolo con Gurobi: x_v in {0,1} per ogni nodo, vincoli x_u + x_v >= 1
per ogni arco (u,v), obiettivo min sum(x_v). Usato nell'esperimento (a) come
termine di paragone ottimale per il Branch-and-Bound custom (vedi
src/main_bb_vs_ilp.cpp, che genera i file grafo e results/bb.csv).

Richiede gurobipy installato e una licenza Gurobi attiva (Named-User
Academic License consigliata: https://www.gurobi.com/academia/).
"""

import argparse
from pathlib import Path

from plot_graph import read_graph

ROOT = Path(__file__).resolve().parent.parent


def solve_ilp(G, time_limit: float):
    import gurobipy as gp
    from gurobipy import GRB

    model = gp.Model("mvc")
    model.Params.OutputFlag = 0
    model.Params.TimeLimit = time_limit

    x = model.addVars(G.nodes(), vtype=GRB.BINARY, name="x")
    for u, v in G.edges():
        model.addConstr(x[u] + x[v] >= 1)
    model.setObjective(gp.quicksum(x[v] for v in G.nodes()), GRB.MINIMIZE)

    model.optimize()

    status = "OPTIMAL" if model.Status == GRB.OPTIMAL else "TIME_LIMIT"
    cardinality = int(round(model.ObjVal)) if model.SolCount > 0 else -1
    return cardinality, model.Runtime, status


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("graph_file", help="file edge-list del grafo (formato SNAP-like)")
    parser.add_argument("-o", "--output", default=str(ROOT / "results" / "ilp.csv"))
    parser.add_argument("--time-limit", type=float, default=180.0)
    parser.add_argument("--label", default=None, help="etichetta del grafo nel CSV (default: nome file)")
    args = parser.parse_args()

    graph_path = Path(args.graph_file)
    G = read_graph(graph_path)
    label = args.label or graph_path.stem

    cardinality, runtime, status = solve_ilp(G, args.time_limit)

    out_path = Path(args.output)
    write_header = not out_path.exists()
    with open(out_path, "a") as f:
        if write_header:
            f.write("N_Nodi,Grafo,Algoritmo,CPU_Time_sec,Cardinalita,Status\n")
        f.write(f"{G.number_of_nodes()},{label},ILP,{runtime:.6f},{cardinality},{status}\n")
    print(f"{label} (n={G.number_of_nodes()}): {runtime:.6f}s, cardinalita={cardinality}, status={status}")


if __name__ == "__main__":
    main()
