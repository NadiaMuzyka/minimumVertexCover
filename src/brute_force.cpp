#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Forza bruta per Minimum Vertex Cover (Sezione 1.3.1 del report): enumera
// tutti i 2^n sottoinsiemi di vertici e restituisce il piu' piccolo che
// copre tutti gli archi. 
vector<int> bruteForceVertexCover(const Graph& G) {
    int n = (int)G.size();

    // Lista di archi (u,v) con u<v: ogni arco compare due volte in Graph
    // (in G[u] e in G[v]), qui lo teniamo una volta sola.
    vector<pair<int,int>> edges;
    for (int u = 0; u < n; u++)
        for (int v : G[u])
            if (v > u) edges.emplace_back(u, v);

    vector<int> best;
    // Enumerazione di tutti i 2^n sottoinsiemi di vertici in binario: il bit i-esimo di mask indica se il vertice i e' incluso nel sottoinsieme.
    //Mask scorre da 0 (nessun vertice incluso) a 2^n-1 (tutti i vertici inclusi).
    // Esempio: n=3, mask=5 (binario 101) corrisponde al sottoinsieme {0,2} di vertici.
    // 1 << n = 1 spostato di n posizioni a sinistra, cioè 2^n.
    for (int mask = 0; mask < (1 << n); mask++) {
        // Potatura banale: se questo sottoinsieme e' gia' grande almeno
        // quanto il migliore trovato finora, non serve nemmeno controllare
        // se e' un cover valido.
        if (!best.empty() && __builtin_popcount(mask) >= (int)best.size())
            continue;

        bool valid = true;
        for (auto [u, v] : edges) {
            // mask >> u sposta il bit u-esimo di mask in posizione 0, e & 1 estrae quel bit: 
            //se e' 1 allora il vertice u e' incluso nel sottoinsieme, altrimenti no.
            if (!((mask >> u) & 1) && !((mask >> v) & 1)) {
                valid = false; // ne' u ne' v sono nel sottoinsieme: arco scoperto
                break;
            }
        }

        if (valid) {
            best.clear();
            for (int i = 0; i < n; i++)
                if ((mask >> i) & 1) best.push_back(i);
        }
    }

    return best;
}
