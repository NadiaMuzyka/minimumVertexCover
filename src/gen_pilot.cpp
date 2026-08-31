#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Uso: ./gen_pilot <n> <avgDegree> <seed> <output_path>\n";
        return 1;
    }
    int n = stoi(argv[1]);
    double avgDegree = stod(argv[2]);
    unsigned seed = (unsigned)stoul(argv[3]);
    string outPath = argv[4];

    Graph G = generateErdosRenyiByAvgDegree(n, avgDegree, seed);
    writeGraphToFile(G, outPath);

    long long m = 0;
    for (auto& adj : G) m += (long long)adj.size();
    m /= 2;
    cout << "Generato " << outPath << ": n=" << n << " m=" << m
         << " avgDegree_target=" << avgDegree
         << " avgDegree_ottenuto=" << fixed << setprecision(2) << (2.0 * m / n)
         << " seed=" << seed << "\n";
    return 0;
}