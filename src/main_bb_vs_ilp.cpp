#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Esperimento (a), parte C++: esegue il Branch-and-Bound custom su un
// insieme di grafi piccoli (dove resta trattabile) e scrive i tempi/
// cardinalita' in results/bb.csv. I grafi sintetici vengono anche salvati
// su file (results/tmp_graph_*.txt) cosi' che scripts/exact_ilp.py possa
// rileggerli e risolverli con Gurobi sugli stessi identici input, per un
// confronto B&B vs ILP alla pari (vedi plan Step 4).

namespace {

struct Instance {
    string label;
    string path; // file da cui exact_ilp.py potra' rileggere lo stesso grafo
    Graph G;
};

} // namespace

int main() {
    vector<Instance> instances;

    // Grafi sintetici a n crescente, densita' moderata: range dove il B&B
    // resta trattabile (fino a ~50 nodi) ma non banale.
    // Nota: il "Grafo" usato come etichetta e' lo stem del nome file (senza
    // estensione), perche' scripts/exact_ilp.py usa Path.stem come etichetta
    // di default quando gli si passa lo stesso file: le due CSV (bb.csv e
    // ilp.csv) devono usare la stessa chiave per poter essere unite in
    // scripts/plot_bb_vs_ilp.py.
    const double avgDegree = 5.0;
    for (int n = 10; n <= 50; n += 5) {
        Graph G = generateErdosRenyiByAvgDegree(n, avgDegree, 50000 + n);
        string stem = "tmp_graph_n" + to_string(n);
        string path = "results/" + stem + ".txt";
        writeGraphToFile(G, path);
        instances.push_back({stem, path, G});
    }

    // Grafi piccoli gia' presenti nel repo (etichetta = stem, come sopra).
    for (const auto& [stem, name] : vector<pair<string,string>>{
             {"small_sparse", "small_sparse.txt"}, {"small_dense", "small_dense.txt"}}) {
        string path = "data/" + name;
        ParsedGraph pg = readGraphFromFile(path);
        instances.push_back({stem, path, pg.G});
    }

    ofstream fout("results/bb.csv");
    fout << "N_Nodi,Grafo,CPU_Time_sec,Cardinalita,Status\n";

    const double timeoutSec = 30.0;
    for (auto& inst : instances) {
        int n = (int)inst.G.size();

        auto t0 = chrono::steady_clock::now();
        ExactResult res = exactVertexCoverStats(inst.G, timeoutSec);
        auto t1 = chrono::steady_clock::now();
        double sec = chrono::duration<double>(t1 - t0).count();

        fout << n << "," << inst.label << "," << fixed << setprecision(6) << sec << ","
             << res.cover.size() << "," << (res.timedOut ? "TIMEOUT" : "OK") << "\n";

        cout << inst.label << " (n=" << n << "): " << sec << "s, cardinalita=" << res.cover.size()
             << (res.timedOut ? " [timeout]" : "") << " -> grafo in " << inst.path << "\n";
    }

    cout << "Risultati B&B scritti in results/bb.csv\n";
    cout << "Esegui scripts/exact_ilp.py su ciascun grafo per ottenere results/ilp.csv\n";
    return 0;
}
