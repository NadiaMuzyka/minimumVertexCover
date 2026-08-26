#include <bits/stdc++.h>
using namespace std;
#include "graph.h"

// Legge un file edge-list stile SNAP ("u v" per riga, righe vuote o che
// iniziano con '#' ignorate) e rimappa gli id arbitrari a indici 0..n-1.
ParsedGraph readGraphFromFile(const string& path) {
    ifstream fin(path);
    if (!fin) {
        cerr << "Cannot open file: " << path << "\n";
        exit(1);
    }

    unordered_map<int,int> id2idx;
    vector<int> idx2id;
    vector<pair<int,int>> edges;
    string line;

    auto getIdx = [&](int raw) -> int {
        auto [it, inserted] = id2idx.emplace(raw, (int)id2idx.size());
        if (inserted) idx2id.push_back(raw);
        return it->second;
    };

    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int u, v;
        if (!(ss >> u >> v)) continue;
        int a = getIdx(u), b = getIdx(v);
        if (a != b)
            edges.emplace_back(a, b);
    }

    int n = (int)id2idx.size();
    ParsedGraph result;
    result.G.assign(n, {});
    for (auto [a, b] : edges) {
        result.G[a].push_back(b);
        result.G[b].push_back(a);
    }
    result.idx2id = std::move(idx2id);
    result.numEdges = (int)edges.size();
    return result;
}

// Scrive un grafo (indici gia' 0..n-1) come edge-list "u v", un arco per
// riga. Usato per far comunicare gli algoritmi C++ con scripts/exact_ilp.py.
void writeGraphToFile(const Graph& G, const string& path) {
    ofstream fout(path);
    if (!fout) {
        cerr << "Cannot write file: " << path << "\n";
        exit(1);
    }
    int n = (int)G.size(), m = 0;
    for (int u = 0; u < n; u++)
        for (int v : G[u])
            if (v > u) m++;

    fout << "# Nodes: " << n << " Edges: " << m << "\n";
    for (int u = 0; u < n; u++)
        for (int v : G[u])
            if (v > u)
                fout << u << " " << v << "\n";
}

// Modello Erdos-Renyi G(n,p): ogni coppia di nodi e' collegata
// indipendentemente con probabilita' p.
Graph generateErdosRenyi(int n, double p, unsigned seed) {
    Graph G(n);
    mt19937 rng(seed);
    uniform_real_distribution<double> dist(0.0, 1.0);
    for (int u = 0; u < n; u++)
        for (int v = u + 1; v < n; v++)
            if (dist(rng) < p) {
                G[u].push_back(v);
                G[v].push_back(u);
            }
    return G;
}

// Variante di generateErdosRenyi parametrizzata sul grado medio atteso
// invece che su p direttamente: p = avgDegree/(n-1), cosi' la densita'
// resta costante al crescere di n (utile per gli esperimenti di scalabilita').
Graph generateErdosRenyiByAvgDegree(int n, double avgDegree, unsigned seed) {
    double p = (n > 1) ? avgDegree / (n - 1) : 0.0;
    p = clamp(p, 0.0, 1.0);
    return generateErdosRenyi(n, p, seed);
}

// Modello Barabasi-Albert: attaccamento preferenziale. Si parte da un nucleo
// di m nodi completamente connessi, poi ogni nuovo nodo si collega a m nodi
// esistenti scelti con probabilita' proporzionale al loro grado.
Graph generateBarabasiAlbert(int n, int m, unsigned seed) {
    m = max(1, min(m, n - 1));
    Graph G(n);
    mt19937 rng(seed);

    vector<int> targets; // un id per ogni "endpoint" di arco esistente
    for (int u = 0; u < m; u++)
        for (int v = u + 1; v < m; v++) {
            G[u].push_back(v);
            G[v].push_back(u);
            targets.push_back(u);
            targets.push_back(v);
        }

    for (int u = m; u < n; u++) {
        unordered_set<int> chosen;
        while ((int)chosen.size() < m && (int)chosen.size() < u) {
            if (targets.empty()) {
                // nucleo senza archi (m=1): scegli uniformemente tra i nodi gia' presenti
                uniform_int_distribution<int> unif(0, u - 1);
                chosen.insert(unif(rng));
            } else {
                uniform_int_distribution<int> unif(0, (int)targets.size() - 1);
                chosen.insert(targets[unif(rng)]);
            }
        }
        for (int v : chosen) {
            G[u].push_back(v);
            G[v].push_back(u);
            targets.push_back(u);
            targets.push_back(v);
        }
    }
    return G;
}
