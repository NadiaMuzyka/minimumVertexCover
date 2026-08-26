#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Exact Minimum Vertex Cover via branch-and-bound (vedi report,
// Capitolo 1, Sezione "Exact Branch-and-Bound").

namespace {

// Lower bound sul numero minimo di vertici necessari a coprire gli archi
// residui: cardinalita' |M| di un matching massimale calcolato greedily
// sul grafo residuo. Dal Teorema 1.1 (report) sappiamo |M| <= |C*|, quindi
// e' un lower bound valido (a differenza di 2|M|, che sovrastimerebbe e
// causerebbe una potatura scorretta).
int lowerBoundMatching(const Graph& G, const vector<bool>& removed) {
    int n = (int)G.size();
    vector<bool> matched(n, false);
    int matchingSize = 0;
    for (int u = 0; u < n; u++) {
        if (removed[u] || matched[u]) continue;
        for (int v : G[u]) {
            if (v <= u || removed[v] || matched[v]) continue;
            matched[u] = true;
            matched[v] = true;
            matchingSize++;
            break;
        }
    }
    return matchingSize;
}

// Trova un arco ancora presente nel grafo residuo, o {-1,-1} se non ce ne sono.
pair<int,int> pickResidualEdge(const Graph& G, const vector<bool>& removed) {
    int n = (int)G.size();
    for (int u = 0; u < n; u++) {
        if (removed[u]) continue;
        for (int v : G[u])
            if (!removed[v]) return {u, v};
    }
    return {-1, -1};
}

void branchAndBound(const Graph& G, vector<bool>& removed, vector<int>& current,
                     vector<int>& best, chrono::steady_clock::time_point deadline,
                     long long& nodesExplored, bool& timedOut) {
    nodesExplored++;
    if (timedOut) return;
    if (chrono::steady_clock::now() >= deadline) { timedOut = true; return; }

    if ((int)current.size() + lowerBoundMatching(G, removed) >= (int)best.size())
        return; // potatura

    auto [u, v] = pickResidualEdge(G, removed);
    if (u == -1) { // nessun arco residuo: current e' un cover valido
        if (current.size() < best.size()) best = current;
        return;
    }

    removed[u] = true;
    current.push_back(u);
    branchAndBound(G, removed, current, best, deadline, nodesExplored, timedOut);
    current.pop_back();
    removed[u] = false;
    if (timedOut) return;

    removed[v] = true;
    current.push_back(v);
    branchAndBound(G, removed, current, best, deadline, nodesExplored, timedOut);
    current.pop_back();
    removed[v] = false;
}

} // namespace

// Upper bound iniziale: soluzione greedy, molto piu' stretta del banale
// best = V dello pseudocodice teorico e quindi piu' efficace nel potare
// da subito rami inutili (ottimizzazione pratica rispetto al report).
ExactResult exactVertexCoverStats(const Graph& G, double timeoutSec) {
    ExactResult result;
    result.cover = greedyVertexCover(G);
    result.nodesExplored = 0;
    result.timedOut = false;

    vector<bool> removed(G.size(), false);
    vector<int> current;
    auto deadline = chrono::steady_clock::now() +
        chrono::duration_cast<chrono::steady_clock::duration>(chrono::duration<double>(timeoutSec));

    branchAndBound(G, removed, current, result.cover, deadline, result.nodesExplored, result.timedOut);
    return result;
}

vector<int> exactVertexCover(const Graph& G) {
    return exactVertexCoverStats(G).cover;
}
