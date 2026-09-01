#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include "../data/graph.h"

static string graphLabel(const string& path) {
    return filesystem::path(path).stem().string();
}

// Uso: ./mvc [path_grafo] [algoritmo: approx|greedy|bb|brute] [output_csv] [timeout_bb_sec]
int main(int argc, char* argv[]) {
    const string path = (argc > 1) ? argv[1] : "data/test.txt";
    const string algo = (argc > 2) ? argv[2] : "approx";
    const string outputCsv = (argc > 3) ? argv[3] : "results/pilot.csv";
    const double timeoutBb = (argc > 4) ? stod(argv[4]) : 300.0;

    ParsedGraph pg = readGraphFromFile(path);
    const Graph& G = pg.G;
    const vector<int>& idx2id = pg.idx2id;

    cout << "Nodes: " << G.size() << "  Edges: " << pg.numEdges << "\n";

    vector<int> cover;
    string status;

    auto t0 = chrono::steady_clock::now();
    if (algo == "approx") { cover = approxVertexCover(G); status = "HEURISTIC"; }
    else if (algo == "greedy") { cover = greedyVertexCover(G); status = "HEURISTIC"; }
    else if (algo == "brute") { cover = bruteForceVertexCover(G); status = "OPTIMAL"; }
    else if (algo == "bb") {
        ExactResult r = exactVertexCoverStats(G, timeoutBb);
        cover = r.cover;
        status = r.timedOut ? "TIME_LIMIT" : "OPTIMAL";
    } else {
        cerr << "Algoritmo sconosciuto: " << algo << " (usa approx|greedy|bb|brute)\n";
        return 1;
    }
    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, milli>(t1 - t0).count();

    cout << "Vertex cover (" << cover.size() << " nodi): ";
    for (int idx : cover) cout << idx2id[idx] << " ";
    cout << "\n";
    cout << "Cardinalita': " << cover.size() << "\n";
    cout << "Tempo: " << fixed << setprecision(3) << ms << " ms\n";
    bool valid = isValidCover(G, cover);
    cout << "Cover valido: " << (valid ? "si" : "NO -- BUG") << "\n";
    if (!valid) {
        cerr << "ERRORE: " << algo << " ha restituito un cover NON valido su " << path << "\n";
        return 2;
    }

    ofstream fout("results/" + algo + "_cover.txt");
    fout << cover.size() << "\n";
    for (size_t i = 0; i < cover.size(); ++i)
        fout << idx2id[cover[i]] << (i + 1 < cover.size() ? ' ' : '\n');

    filesystem::create_directories(filesystem::path(outputCsv).parent_path());
    bool writeHeader = !filesystem::exists(outputCsv);
    ofstream csv(outputCsv, ios::app);
    if (writeHeader)
        csv << "N_Nodi,Grafo,Algoritmo,CPU_Time_sec,Cardinalita,Status\n";
    csv << G.size() << "," << graphLabel(path) << "," << algo << ","
        << fixed << setprecision(6) << (ms / 1000.0) << ","
        << cover.size() << "," << status << "\n";

    return 0;
}