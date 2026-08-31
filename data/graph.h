#ifndef GRAPH_H  // Header Guard: impedisce di includerlo due volte
#define GRAPH_H

#include <vector>
#include <string>

// Definiamo il tipo Graph una volta sola per tutto il progetto.
// Rappresentazione a lista di adiacenza: G[u] contiene i vicini di u.
typedef std::vector<std::vector<int>> Graph;

// --- Funzioni di Utilità (utils.cpp) ---

// Risultato del parsing di un file edge-list stile SNAP: gli id originali
// (arbitrari) vengono rimappati a indici 0..n-1; idx2id permette di
// ritradurli per l'output.
struct ParsedGraph {
    Graph G;
    std::vector<int> idx2id;
    int numEdges;
};

ParsedGraph readGraphFromFile(const std::string& path);
void writeGraphToFile(const Graph& G, const std::string& path);

// Generatori sintetici (per esperimenti di scalabilita'/NP-hardness).
Graph generateErdosRenyi(int n, double p, unsigned seed);
Graph generateErdosRenyiByAvgDegree(int n, double avgDegree, unsigned seed);
Graph generateBarabasiAlbert(int n, int m, unsigned seed);

// --- Algoritmi (staranno nei rispettivi .cpp) ---
std::vector<int> approxVertexCover(const Graph& G);
std::vector<int> greedyVertexCover(const Graph& G);
std::vector<int> exactVertexCover(const Graph& G);
std::vector<int> bruteForceVertexCover(const Graph& G);

// Forza bruta: enumera tutti i 2^n sottoinsiemi di vertici (Sezione 1.3.1
// del report). Puramente di riferimento/verifica, non usata negli
// esperimenti (sostituita dal confronto B&B vs ILP): pratico solo per
// n molto piccoli (indicativamente n <= 20-25).
std::vector<int> bruteForceVertexCover(const Graph& G);

// Variante strumentata del branch-and-bound: espone nodi esplorati (per
// misurare l'effetto della potatura) e se il timeout e' scattato prima di
// dimostrare l'ottimalita' (usata dagli esperimenti in src/main_*.cpp).
struct ExactResult {
    std::vector<int> cover;
    long long nodesExplored;
    bool timedOut;
};
ExactResult exactVertexCoverStats(const Graph& G, double timeoutSec = 30.0);

#endif