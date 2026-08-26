#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Esperimento (c): scalabilita' al crescere di n, a densita' costante
// (Erdos-Renyi con grado medio fissato, p = avgDegree/(n-1)).
// Approx e Greedy scalano su n grandi; il Branch-and-Bound, esponenziale nel
// caso peggiore, viene testato solo su n piccoli con un timeout di sicurezza.
// Scrive results/scalability.csv.

namespace {

ofstream fout;

void logRow(int n, const string& topologia, const string& algo, int card, double sec, const string& status) {
    fout << n << "," << topologia << "," << algo << "," << card << ","
         << fixed << setprecision(6) << sec << "," << status << "\n";
}

} // namespace

int main() {
    fout.open("results/scalability.csv");
    fout << "N_Nodi,Topologia,Algoritmo,Cardinalita,CPU_Time_sec,Status\n";

    const double avgDegree = 6.0;
    const string topologia = "Erdos-Renyi (deg=6)";

    // Approx e Greedy: n grandi, raddoppiando ad ogni passo.
    for (int n = 100; n <= 12800; n *= 2) {
        Graph G = generateErdosRenyiByAvgDegree(n, avgDegree, 10000 + n);

        auto t0 = chrono::steady_clock::now();
        vector<int> cover = approxVertexCover(G);
        auto t1 = chrono::steady_clock::now();
        logRow(n, topologia, "Approx", (int)cover.size(), chrono::duration<double>(t1 - t0).count(), "OK");

        t0 = chrono::steady_clock::now();
        cover = greedyVertexCover(G);
        t1 = chrono::steady_clock::now();
        logRow(n, topologia, "Greedy", (int)cover.size(), chrono::duration<double>(t1 - t0).count(), "OK");

        cout << "n=" << n << " (approx/greedy) completato\n";
    }

    // Branch-and-Bound: n piccoli, con timeout di sicurezza.
    const double bbTimeoutSec = 5.0;
    for (int n = 8; n <= 26; n += 2) {
        Graph G = generateErdosRenyiByAvgDegree(n, avgDegree, 20000 + n);

        auto t0 = chrono::steady_clock::now();
        ExactResult res = exactVertexCoverStats(G, bbTimeoutSec);
        auto t1 = chrono::steady_clock::now();
        logRow(n, topologia, "BB", (int)res.cover.size(), chrono::duration<double>(t1 - t0).count(),
               res.timedOut ? "TIMEOUT" : "OK");

        cout << "n=" << n << " (bb) completato" << (res.timedOut ? " [timeout]" : "") << "\n";
    }

    cout << "Risultati scritti in results/scalability.csv\n";
    return 0;
}
