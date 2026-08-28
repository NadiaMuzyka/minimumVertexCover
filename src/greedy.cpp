#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Euristica greedy per Vertex Cover: ad ogni passo sceglie il vertice di grado
// massimo nel grafo residuo, lo aggiunge al cover e rimuove lui e i suoi archi.
// Tempo di esecuzione: O(|V|^2 + |E|).
vector<int> greedyVertexCover(const Graph& G) {
    int n = (int)G.size(); //n = |V|
    vector<bool> removed(n, false); // vettore di booleani che indica se un vertice è stato rimosso o meno
    vector<int> deg(n); // deg[v] = grado residuo di v nel grafo residuo
    int remainingEdges = 0;

    for (int u = 0; u < n; u++) {
        deg[u] = (int)G[u].size(); // grado residuo di u
        remainingEdges += deg[u]; // numero di archi residui (ogni arco e' contato due volte)
    }
    remainingEdges /= 2; // ogni arco e' contato due volte (una per estremo)

    vector<int> cover;

    while (remainingEdges > 0) {
        int u = -1; //così alla prima iterazione il vertice di grado massimo sarà il primo vertice incontrato non rimosso

        // Trova il vertice di grado massimo nel grafo residuo
        for (int v = 0; v < n; v++)
            if (!removed[v] && (u == -1 || deg[v] > deg[u]))
                u = v;

        cover.push_back(u);
        removed[u] = true;

        // Aggiorna i gradi dei vicini di u
        for (int w : G[u]) {
            if (!removed[w]) {
                deg[w]--;
                remainingEdges--;
            }
        }
    }

    return cover;
}
