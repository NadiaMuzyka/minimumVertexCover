#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include <sys/wait.h>
#include "../data/graph.h"

// Esperimento 2: trova il muro esponenziale dell'ILP (Gurobi), separatamente
// per grafi sparsi e densi. Stessa tecnica di experiment_bb_wall: doubling
// con early stopping (DOE, Guideline 2.7) -- si parte da n0, si raddoppia,
// ci si ferma un passo dopo il primo TIME_LIMIT.
//
// A differenza di experiment_bb_wall, l'ILP non e' in-process (Gurobi e'
// accessibile solo via Python): ogni punto viene delegato a
// scripts/exact_ilp.py via system(), poi lo stato (OPTIMAL/TIME_LIMIT) viene
// riletto dal CSV di output per decidere se continuare a raddoppiare.
//
// Uso (da lanciare dalla root del progetto, con il venv gia' attivato):
//   ./experiment_ilp_wall --n0 10 --densities sparse dense --reps 3
//                          --timeout 120 --confirm-steps 1 --max-doublings 15
//                          --output results/exp2_ilp_wall.csv
//                          --graph-dir data/exp2 [--skip-existing]

struct Args {
    int n0 = 10;
    vector<string> densities = {"sparse", "dense"};
    int reps = 3;
    int seedBase = 1000;
    double timeout = 120.0;
    int confirmSteps = 1;
    int maxDoublings = 15;
    string output = "results/exp2_ilp_wall.csv";
    string graphDir = "data/exp2";
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

static string readStatus(const string& path, const string& label) {
    ifstream fin(path);
    string line, status;
    getline(fin, line);
    while (getline(fin, line)) {
        stringstream ss(line);
        string nNodi, grafo, algo, time, card, st;
        getline(ss, nNodi, ',');
        getline(ss, grafo, ',');
        getline(ss, algo, ',');
        getline(ss, time, ',');
        getline(ss, card, ',');
        getline(ss, st, ',');
        if (grafo == label && algo == "ILP") status = st;
    }
    return status;
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

                if (!done.count({label, "ILP"})) {
                    string cmd = "python3 scripts/exact_ilp.py " + graphPath +
                                 " --time-limit " + to_string(args.timeout) +
                                 " --output " + args.output;
                    int rc = system(cmd.c_str());
                    if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
                        cerr << "ERRORE: exact_ilp.py e' fallito su " << graphPath << "\n";
                        return 3;
                    }
                }

                string status = readStatus(args.output, label);
                if (status == "TIME_LIMIT") anyTimeout = true;
                cout << "  n=" << n << " seed=" << seed << ": " << status << endl;
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

        if (wallStep == -1) {
            cout << "Nessun muro trovato per " << density << " entro n=" << n
                 << " (" << args.maxDoublings << " raddoppi)" << endl;
        }
    }

    cout << "Fatto. Risultati in " << args.output << "\n";
    return 0;
}