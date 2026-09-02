#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include <sys/wait.h>
#include "../data/graph.h"
#include "experiment_common.h"

// Esperimento finale: qualita'. L'obiettivo è capire di quanto si discostano 2-approssimato e
// greedy dall'ottimo 
//
// Uso (da lanciare dalla root del progetto, con il venv gia' attivato):
//   ./experiment_quality --n-values 10 20 30 ... 200 --densities sparse dense
//                         --reps 5 --ilp-timeout 120
//                         --output results/exp_quality.csv
//                         --graph-dir data/exp_quality
// Se --n-values non e' specificato, il default e' 10,20,...,200.

struct Args {
    vector<int> nValues;                             // griglia fissa di n da testare (default: 10,20,...,200)
    vector<string> densities = {"sparse", "dense"}; // livelli da testare, uno per volta
    int reps = 5;                                    // ripetizioni (grafi diversi) per ogni (n, densita')
    int seedBase = 1000;                             // seed della ripetizione 0; le altre sono seedBase+rep
    double ilpTimeout = 120.0;                       // limite di tempo passato a Gurobi (--time-limit), in secondi
    string output = "results/exp_quality.csv";       // CSV condiviso, righe di approx/greedy/ILP mescolate
    string graphDir = "data/exp_quality";             // dove salvare i grafi generati, per riproducibilita'
};

// Legge i flag da riga di comando sovrascrivendo i default di Args.
// --n-values e --densities sono a valore multiplo (si veda collectValues).
static Args parseArgs(int argc, char* argv[]) {
    Args a;
    for (int i = 1; i < argc; i++) {
        string flag = argv[i];
        if (flag == "--n-values") {
            a.nValues.clear();
            for (auto& v : collectValues(i, argc, argv)) a.nValues.push_back(stoi(v));
        } else if (flag == "--densities") {
            a.densities.clear();
            for (auto& v : collectValues(i, argc, argv)) a.densities.push_back(v);
        } else if (flag == "--reps") a.reps = stoi(argv[++i]);
        else if (flag == "--seed-base") a.seedBase = stoi(argv[++i]);
        else if (flag == "--ilp-timeout") a.ilpTimeout = stod(argv[++i]);
        else if (flag == "--output") a.output = argv[++i];
        else if (flag == "--graph-dir") a.graphDir = argv[++i];
        else { cerr << "Argomento sconosciuto: " << flag << "\n"; exit(1); }
    }
    if (a.nValues.empty())
        for (int n = 10; n <= 200; n += 10) a.nValues.push_back(n);
    return a;
}

// Esegue approx, greedy (in-process) e ILP (sottoprocesso Python) sullo
// STESSO grafo generato una sola volta, scrivendo tutte e tre le righe nel
// CSV: e' quello che rende confrontabile il rapporto cardinalita'/OPT dei
// tre algoritmi punto per punto, invece che su istanze diverse.
static void runTrial(int n, const string& density, int seed, double avgDeg, const Args& args) {
    string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);
    string graphPath = args.graphDir + "/" + label + ".txt";

    Graph G = generateErdosRenyiByAvgDegree(n, avgDeg, (unsigned)seed);
    writeGraphToFile(G, graphPath); // serve a exact_ilp.py (Python) per leggerlo

    // 2-approssimato: controllo di sanita' sul cover, poi riga nel CSV.
    auto t0 = chrono::steady_clock::now();
    vector<int> coverA = approxVertexCover(G);
    auto t1 = chrono::steady_clock::now();
    if (!isValidCover(G, coverA)) {
        cerr << "ERRORE: approx ha restituito un cover NON valido su " << label << "\n";
        exit(2);
    }
    appendCsvRow(args.output, n, label, "approx",
                 chrono::duration<double>(t1 - t0).count(), (int)coverA.size(), "HEURISTIC");

    // Greedy: stessa procedura, sullo stesso grafo G.
    auto t2 = chrono::steady_clock::now();
    vector<int> coverG = greedyVertexCover(G);
    auto t3 = chrono::steady_clock::now();
    if (!isValidCover(G, coverG)) {
        cerr << "ERRORE: greedy ha restituito un cover NON valido su " << label << "\n";
        exit(2);
    }
    appendCsvRow(args.output, n, label, "greedy",
                 chrono::duration<double>(t3 - t2).count(), (int)coverG.size(), "HEURISTIC");

    // ILP
    string cmd = "python3 scripts/exact_ilp.py " + graphPath +
                 " --time-limit " + to_string(args.ilpTimeout) +
                 " --output " + args.output;
    int rc = system(cmd.c_str());
    if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
        cerr << "ERRORE: exact_ilp.py e' fallito su " << graphPath << "\n";
        exit(3);
    }
}

int main(int argc, char* argv[]) {
    Args args = parseArgs(argc, argv);
    filesystem::create_directories(args.graphDir);
    filesystem::create_directories(filesystem::path(args.output).parent_path());

    int total = (int)args.nValues.size() * (int)args.densities.size() * args.reps;
    int count = 0;

    for (int n : args.nValues) {
        for (const string& density : args.densities) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);

            for (int rep = 0; rep < args.reps; rep++) {
                int seed = args.seedBase + rep;
                count++;
                cout << "[" << count << "/" << total << "] n" << n << "_" << density
                     << "_s" << seed << endl;
                runTrial(n, density, seed, avgDeg, args);
            }
        }
    }

    cout << "Fatto. Risultati in " << args.output << "\n";
    return 0;
}