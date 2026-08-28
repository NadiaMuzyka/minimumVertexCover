#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// 2-approximation for Minimum Vertex Cover via maximal matching. O(|V|+|E|).
vector<int> approxVertexCover(const Graph& G) {
    int n = (int)G.size(); //n = |V|
    vector<bool> covered(n, false); //vettore di booleani che indica se un vertice è coperto o meno
    vector<int> cover; // C ← ∅
    //E' è rappresentato implicitamente tramite un array covered, evitando il costo di rimozione fisica degli archi
    for (int u = 0; u < n; u++) {
        for (int v : G[u]) {
            if (v <= u) continue; // ogni arco (u,v) e' in G[u] e in G[v]: lo consideriamo una sola volta
            if (!covered[u] && !covered[v]) { //L'arco (a,b) è ancora in E'" ⟺ !covered[a] && !covered[b]
                covered[u] = true;
                covered[v] = true;
                cover.push_back(u);
                cover.push_back(v);
            }
        }
    }

    return cover;
}


