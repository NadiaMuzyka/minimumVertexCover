#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Euristica greedy per Vertex Cover: ad ogni passo sceglie il vertice di grado
// massimo nel grafo residuo, lo aggiunge al cover e rimuove lui e i suoi archi.
// Tempo di esecuzione: O(|V|^2 + |E|).
vector<int> greedyVertexCover(const Graph& G) {
    int n = (int)G.size();
    vector<bool> removed(n, false);
    vector<int> deg(n);
    int remainingEdges = 0;

    for (int u = 0; u < n; u++) {
        deg[u] = (int)G[u].size();
        remainingEdges += deg[u];
    }
    remainingEdges /= 2; // ogni arco e' contato due volte (una per estremo)

    vector<int> cover;

    while (remainingEdges > 0) {
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!removed[v] && (u == -1 || deg[v] > deg[u]))
                u = v;

        cover.push_back(u);
        removed[u] = true;

        for (int w : G[u]) {
            if (!removed[w]) {
                deg[w]--;
                remainingEdges--;
            }
        }
    }

    return cover;
}
