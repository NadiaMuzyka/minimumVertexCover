#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include "../data/graph.h"

// Esperimento 1: trova il muro esponenziale del B&B, separatamente per
// grafi sparsi e densi (i due muri sono troppo distanti per condividere lo
// stesso range). Tecnica: doubling experiment con early stopping (DOE,
// Guideline 2.7) -- si parte da n0, si raddoppia ad ogni passo, e ci si
// ferma da soli un passo dopo il primo TIME_LIMIT, invece di usare una
// lista di n decisa a tavolino.
//
// Uso:
//   ./experiment_bb_wall --n0 10 --densities sparse dense --reps 3
//                         --timeout 120 --confirm-steps 1 --max-doublings 15
//                         --output results/exp1_bb_wall.csv
//                         --graph-dir data/exp1 [--skip-existing]

struct Args {
    int n0 = 10;
    vector<string> densities = {"sparse", "dense"};
    int reps = 3;
    int seedBase = 1000;
    double timeout = 120.0;
    int confirmSteps = 1;
    int maxDoublings = 15;
    string output = "results/exp1_bb_wall.csv";
    string graphDir = "data/exp1";
    bool skipExisting = false;
};

static double sparseAvgDegree(int n) { return round(2.0 * log((double)n) * 100.0) / 100.0; }
static double denseAvgDegree(int n) { return round((n / 2.0) * 100.0) / 100.0; }

static vector<string> collectValues(int& i, int argc, char* argv[]) {
    vector<string> vals;
    while (i + 1 < argc && string(argv[i + 1]).rfind("--", 0) != 0) {
        vals.push_back(argv[++i]);
    }
    return vals;
}

static Args parseArgs(int argc, char* argv[]) {
    Args a;
    for (int i = 1; i < argc; i++) {
        string flag = argv[i];
        if (flag == "--n0") a.n0 = stoi(argv[++i]);
        else if (flag == "--densities") {
            a.densities.clear();
            for (auto& v : collectValues(i, argc, argv)) a.densities.push_back(v);
        } else if (flag == "--reps") a.reps = stoi(argv[++i]);
        else if (flag == "--seed-base") a.seedBase = stoi(argv[++i]);
        else if (flag == "--timeout") a.timeout = stod(argv[++i]);
        else if (flag == "--confirm-steps") a.confirmSteps = stoi(argv[++i]);
        else if (flag == "--max-doublings") a.maxDoublings = stoi(argv[++i]);
        else if (flag == "--output") a.output = argv[++i];
        else if (flag == "--graph-dir") a.graphDir = argv[++i];
        else if (flag == "--skip-existing") a.skipExisting = true;
        else { cerr << "Argomento sconosciuto: " << flag << "\n"; exit(1); }
    }
    return a;
}

static set<pair<string,string>> loadDone(const string& path) {
    set<pair<string,string>> done;
    ifstream fin(path);
    if (!fin) return done;
    string line;
    getline(fin, line);
    while (getline(fin, line)) {
        stringstream ss(line);
        string nNodi, grafo, algo;
        getline(ss, nNodi, ',');
        getline(ss, grafo, ',');
        getline(ss, algo, ',');
        done.insert({grafo, algo});
    }
    return done;
}

static void appendCsvRow(const string& path, int n, const string& label, const string& algo,
                          double sec, int cardinality, const string& status) {
    bool writeHeader = !filesystem::exists(path);
    ofstream csv(path, ios::app);
    if (writeHeader)
        csv << "N_Nodi,Grafo,Algoritmo,CPU_Time_sec,Cardinalita,Status\n";
    csv << n << "," << label << "," << algo << "," << fixed << setprecision(6) << sec
        << "," << cardinality << "," << status << "\n";
}

int main(int argc, char* argv[]) {
    Args args = parseArgs(argc, argv);
    filesystem::create_directories(args.graphDir);
    filesystem::create_directories(filesystem::path(args.output).parent_path());

    set<pair<string,string>> done;
    if (args.skipExisting) done = loadDone(args.output);

    for (const string& density : args.densities) {
        cout << "=== Densita': " << density << " ===" << endl;
        int n = args.n0;
        int wallStep = -1;

        for (int step = 0; step < args.maxDoublings; step++) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);
            bool anyTimeout = false;

            for (int rep = 0; rep < args.reps; rep++) {
                int seed = args.seedBase + rep;
                string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);
                string graphPath = args.graphDir + "/" + label + ".txt";

                Graph G = generateErdosRenyiByAvgDegree(n, avgDeg, (unsigned)seed);
                writeGraphToFile(G, graphPath);

                if (!done.count({label, "bb"})) {
                    auto t0 = chrono::steady_clock::now();
                    ExactResult r = exactVertexCoverStats(G, args.timeout);
                    auto t1 = chrono::steady_clock::now();
                    double sec = chrono::duration<double>(t1 - t0).count();

                    if (!isValidCover(G, r.cover)) {
                        cerr << "ERRORE: B&B ha restituito un cover NON valido su " << graphPath << "\n";
                        return 2;
                    }
                    string status = r.timedOut ? "TIME_LIMIT" : "OPTIMAL";
                    if (r.timedOut) anyTimeout = true;
                    appendCsvRow(args.output, n, label, "bb", sec, (int)r.cover.size(), status);
                    cout << "  n=" << n << " seed=" << seed << ": " << fixed << setprecision(3)
                         << sec << "s (" << status << ")" << endl;
                }
            }

            if (anyTimeout && wallStep == -1) {
                wallStep = 0;
                cout << ">>> Muro trovato per " << density << " a n=" << n << endl;
            } else if (wallStep >= 0) {
                wallStep++;
            }
            if (wallStep >= args.confirmSteps) break;

            n *= 2;
        }
    }

    cout << "Fatto. Risultati in " << args.output << "\n";
    return 0;
}