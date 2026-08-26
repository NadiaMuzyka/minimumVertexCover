#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Uso: ./mvc [path_grafo] [algoritmo: approx|greedy|bb]
int main(int argc, char* argv[]) {
    const string path = (argc > 1) ? argv[1] : "data/test.txt";
    const string algo = (argc > 2) ? argv[2] : "approx";

    ParsedGraph pg = readGraphFromFile(path);
    const Graph& G = pg.G;
    const vector<int>& idx2id = pg.idx2id;

    cout << "Nodes: " << G.size() << "  Edges: " << pg.numEdges << "\n";

    vector<int> cover;
    auto t0 = chrono::steady_clock::now();
    if (algo == "approx") cover = approxVertexCover(G);
    else if (algo == "greedy") cover = greedyVertexCover(G);
    else if (algo == "bb") cover = exactVertexCover(G);
    else {
        cerr << "Algoritmo sconosciuto: " << algo << " (usa approx|greedy|bb)\n";
        return 1;
    }
    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, milli>(t1 - t0).count();

    cout << "Vertex cover (" << cover.size() << " nodi): ";
    for (int idx : cover) cout << idx2id[idx] << " ";
    cout << "\n";
    cout << "Cardinalita': " << cover.size() << "\n";
    cout << "Tempo: " << fixed << setprecision(3) << ms << " ms\n";

    ofstream fout("results/" + algo + "_cover.txt");
    fout << cover.size() << "\n";
    for (size_t i = 0; i < cover.size(); ++i)
        fout << idx2id[cover[i]] << (i + 1 < cover.size() ? ' ' : '\n');

    return 0;
}
