#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Esperimento (b): confronto qualitativo Greedy vs 2-Approximation.
// Per ciascun grafo di test esegue entrambi gli algoritmi, misura tempo e
// cardinalita' del cover prodotto; dove il grafo e' abbastanza piccolo,
// calcola anche l'ottimo (via branch-and-bound) per il rapporto di
// approssimazione osservato. Scrive results/quality.csv.

namespace {

struct GraphInstance {
    string topologia;
    string dataset;
    Graph G;
    bool tryOptimal; // se true, tenta di calcolare l'ottimo con un B&B a timeout breve
};

int edgeCount(const Graph& G) {
    long long m = 0;
    for (auto& adj : G) m += (long long)adj.size();
    return (int)(m / 2);
}

} // namespace

int main() {
    vector<GraphInstance> instances;

    // Grafo reale
    {
        ParsedGraph pg = readGraphFromFile("data/CA-GrQc.txt");
        instances.push_back({"Reale (SNAP)", "CA-GrQc", pg.G, false});
    }

    // File sintetici gia' presenti nel repo, abbastanza piccoli da avere l'ottimo
    for (const string name : {"small_sparse.txt", "small_dense.txt"}) {
        ParsedGraph pg = readGraphFromFile("data/" + name);
        instances.push_back({"Sintetico (file)", name, pg.G, true});
    }

    // Erdos-Renyi a densita'/n variabili (troppo grandi per l'ottimo esatto)
    for (int n : {100, 300, 1000}) {
        for (double avgDeg : {4.0, 10.0}) {
            Graph G = generateErdosRenyiByAvgDegree(n, avgDeg, 1000 + n + (int)avgDeg);
            instances.push_back({"Erdos-Renyi", "ER_n" + to_string(n) + "_deg" + to_string((int)avgDeg), G, false});
        }
    }

    // Barabasi-Albert a n variabile
    for (int n : {100, 300, 1000}) {
        Graph G = generateBarabasiAlbert(n, 3, 2000 + n);
        instances.push_back({"Barabasi-Albert", "BA_n" + to_string(n) + "_m3", G, false});
    }

    // Erdos-Renyi piccoli, ottimo calcolabile
    for (int n : {12, 16, 20}) {
        Graph G = generateErdosRenyiByAvgDegree(n, 4.0, 3000 + n);
        instances.push_back({"Erdos-Renyi (piccolo)", "ER_small_n" + to_string(n), G, true});
    }

    ofstream fout("results/quality.csv");
    fout << "N_Nodi,Topologia,Dataset,Algoritmo,Cardinalita,CPU_Time_sec,Cardinalita_Ottima_se_nota\n";

    for (auto& inst : instances) {
        int n = (int)inst.G.size();

        string optimalStr = "";
        if (inst.tryOptimal) {
            ExactResult res = exactVertexCoverStats(inst.G, 10.0);
            if (!res.timedOut) optimalStr = to_string(res.cover.size());
        }

        auto runAndLog = [&](const string& algoName, const vector<int>& cover, double sec) {
            fout << n << "," << inst.topologia << "," << inst.dataset << "," << algoName << ","
                 << cover.size() << "," << fixed << setprecision(6) << sec << ","
                 << optimalStr << "\n";
        };

        auto t0 = chrono::steady_clock::now();
        vector<int> approxCover = approxVertexCover(inst.G);
        auto t1 = chrono::steady_clock::now();
        runAndLog("Approx", approxCover, chrono::duration<double>(t1 - t0).count());

        t0 = chrono::steady_clock::now();
        vector<int> greedyCover = greedyVertexCover(inst.G);
        t1 = chrono::steady_clock::now();
        runAndLog("Greedy", greedyCover, chrono::duration<double>(t1 - t0).count());

        cout << "Completato: " << inst.dataset << " (n=" << n << ", m=" << edgeCount(inst.G) << ")\n";
    }

    cout << "Risultati scritti in results/quality.csv\n";
    return 0;
}
