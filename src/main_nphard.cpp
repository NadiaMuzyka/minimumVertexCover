#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

// Esperimento (d): muro esponenziale (NP-hardness) del Branch-and-Bound.
// Grafi piccoli ma di dimensione crescente, densita' alta (p=0.5) per
// rendere il problema difficile anche a n modesti. A differenza del
// generatore di esperimenti analogo del collega (max-cut-ae), qui e'
// presente un timeout esplicito per evitare run bloccate: una volta
// raggiunto, l'esperimento su quella topologia si interrompe (n successivi
// sarebbero solo piu' difficili). Registra anche i nodi esplorati
// dell'albero di ricerca, per mostrare concretamente l'effetto della
// potatura rispetto al caso peggiore O(2^n). Scrive results/nphard.csv.

namespace {

void runSeries(ofstream& fout, const string& topologia, double timeoutSec,
               function<Graph(int)> makeGraph) {
    for (int n = 10; n <= 28; n += 2) {
        Graph G = makeGraph(n);

        auto t0 = chrono::steady_clock::now();
        ExactResult res = exactVertexCoverStats(G, timeoutSec);
        auto t1 = chrono::steady_clock::now();
        double sec = chrono::duration<double>(t1 - t0).count();

        fout << n << "," << topologia << ",BB," << res.cover.size() << ","
             << fixed << setprecision(6) << sec << "," << res.nodesExplored << ","
             << (res.timedOut ? "TIMEOUT" : "OK") << "\n";

        cout << topologia << " n=" << n << ": " << sec << "s, nodi esplorati="
             << res.nodesExplored << (res.timedOut ? " [timeout]" : "") << "\n";

        if (res.timedOut) {
            cout << topologia << ": timeout raggiunto, interrompo la crescita di n.\n";
            break;
        }
    }
}

} // namespace

int main() {
    ofstream fout("results/nphard.csv");
    fout << "N_Nodi,Topologia,Algoritmo,Cardinalita,CPU_Time_sec,NodiEsplorati,Status\n";

    const double timeoutSec = 20.0;

    runSeries(fout, "Erdos-Renyi (p=0.5)", timeoutSec, [](int n) {
        return generateErdosRenyi(n, 0.5, 30000 + n);
    });

    runSeries(fout, "Barabasi-Albert", timeoutSec, [](int n) {
        int m = max(1, n / 4);
        return generateBarabasiAlbert(n, m, 40000 + n);
    });

    cout << "Risultati scritti in results/nphard.csv\n";
    return 0;
}
