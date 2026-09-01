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
    int declaredN = -1;

    auto getIdx = [&](int raw) -> int {
        auto [it, inserted] = id2idx.emplace(raw, (int)id2idx.size());
        if (inserted) idx2id.push_back(raw);
        return it->second;
    };

    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') {
            int n_hdr, m_hdr;
            if (sscanf(line.c_str(), "# Nodes: %d Edges: %d", &n_hdr, &m_hdr) == 2)
                declaredN = n_hdr;
            continue;
        }
        istringstream ss(line);
        int u, v;
        if (!(ss >> u >> v)) continue;
        int a = getIdx(u), b = getIdx(v);
        if (a != b)
            edges.emplace_back(a, b);
    }

    if (declaredN > (int)id2idx.size()) {
        bool idsFitDeclaredRange = true;
        for (auto& [raw, idx] : id2idx)
            if (raw < 0 || raw >= declaredN) { idsFitDeclaredRange = false; break; }
        if (idsFitDeclaredRange)
            for (int v = 0; v < declaredN; v++)
                getIdx(v);
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

// Stesso modello, parametrizzato sul grado medio atteso invece che su p:
// piu' comodo per fissare i design point (Capitolo 3), dove i due livelli
// sparso/denso sono pensati in termini di grado medio, non di probabilita'.
// In G(n,p) il grado atteso di un nodo e' (n-1)*p, quindi si ricava
// p = avgDegree / (n-1) e si riusa la funzione sopra.
Graph generateErdosRenyiByAvgDegree(int n, double avgDegree, unsigned seed) {
    double p = (n > 1) ? avgDegree / (n - 1) : 0.0;
    p = min(1.0, max(0.0, p)); // clamp di sicurezza se avgDegree > n-1
    return generateErdosRenyi(n, p, seed);
}

bool isValidCover(const Graph& G, const vector<int>& cover) {
    vector<bool> inCover(G.size(), false);
    for (int v : cover) {
        if (v < 0 || v >= (int)G.size()) return false;
        inCover[v] = true;
    }
    for (int u = 0; u < (int)G.size(); u++)
        for (int v : G[u])
            if (v > u && !inCover[u] && !inCover[v])
                return false;
    return true;
}

Graph generateErdosRenyiFast(int n, double p, unsigned seed) {
    Graph G(n);
    if (p <= 0.0 || n < 2) return G;
    if (p >= 1.0) p = 1.0 - 1e-15;
    mt19937_64 rng(seed);
    uniform_real_distribution<double> dist(0.0, 1.0);
    double logNotP = log(1.0 - p);

    long long i = 0, j = -1;
    while (i < n) {
        double r = dist(rng);
        j += 1 + (long long)floor(log(1.0 - r) / logNotP);
        while (j >= i && i < n) {
            j -= i;
            i++;
        }
        if (i < n) {
            G[i].push_back((int)j);
            G[(int)j].push_back((int)i);
        }
    }
    return G;
}

Graph generateErdosRenyiFastByAvgDegree(int n, double avgDegree, unsigned seed) {
    double p = (n > 1) ? avgDegree / (n - 1) : 0.0;
    p = min(1.0, max(0.0, p));
    return generateErdosRenyiFast(n, p, seed);
}
