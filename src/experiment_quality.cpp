#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include <sys/wait.h>
#include "../data/graph.h"

// Esperimento finale: qualita' -- di quanto si discostano 2-approssimato e
// greedy dall'ottimo (ILP, verita' di riferimento: l'esperimento 2 ha gia'
// mostrato che il suo muro e' a n=320, quindi su questo range n=10..200
// risolve sempre, mai in TIME_LIMIT).
//
// A differenza dei muri e della crescita, qui non serve una logica
// adattiva: e' una griglia fissa di n, la stessa identica su cui si vuole
// vedere come cambia il rapporto cardinalita'/OPT.
//
// Uso (da lanciare dalla root del progetto, con il venv gia' attivato):
//   ./experiment_quality --n-values 10 20 30 ... 200 --densities sparse dense
//                         --reps 5 --ilp-timeout 120
//                         --output results/exp_quality.csv
//                         --graph-dir data/exp_quality [--skip-existing]
// Se --n-values non e' specificato, il default e' 10,20,...,200.

struct Args {
    vector<int> nValues;
    vector<string> densities = {"sparse", "dense"};
    int reps = 5;
    int seedBase = 1000;
    double ilpTimeout = 120.0;
    string output = "results/exp_quality.csv";
    string graphDir = "data/exp_quality";
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
        else if (flag == "--skip-existing") a.skipExisting = true;
        else { cerr << "Argomento sconosciuto: " << flag << "\n"; exit(1); }
    }
    if (a.nValues.empty())
        for (int n = 10; n <= 200; n += 10) a.nValues.push_back(n);
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
        getline(ss, nNodi, ','); getline(ss, grafo, ','); getline(ss, algo, ',');
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

    int total = (int)args.nValues.size() * (int)args.densities.size() * args.reps;
    int count = 0;

    for (int n : args.nValues) {
        for (const string& density : args.densities) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);

            for (int rep = 0; rep < args.reps; rep++) {
                int seed = args.seedBase + rep;
                string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);
                string graphPath = args.graphDir + "/" + label + ".txt";
                count++;
                cout << "[" << count << "/" << total << "] " << label << endl;

                Graph G = generateErdosRenyiByAvgDegree(n, avgDeg, (unsigned)seed);
                writeGraphToFile(G, graphPath); // serve a exact_ilp.py (Python) per leggerlo

                if (!done.count({label, "approx"})) {
                    auto t0 = chrono::steady_clock::now();
                    vector<int> cover = approxVertexCover(G);
                    auto t1 = chrono::steady_clock::now();
                    double sec = chrono::duration<double>(t1 - t0).count();
                    if (!isValidCover(G, cover)) {
                        cerr << "ERRORE: approx ha restituito un cover NON valido su " << label << "\n";
                        return 2;
                    }
                    appendCsvRow(args.output, n, label, "approx", sec, (int)cover.size(), "HEURISTIC");
                }

                if (!done.count({label, "greedy"})) {
                    auto t0 = chrono::steady_clock::now();
                    vector<int> cover = greedyVertexCover(G);
                    auto t1 = chrono::steady_clock::now();
                    double sec = chrono::duration<double>(t1 - t0).count();
                    if (!isValidCover(G, cover)) {
                        cerr << "ERRORE: greedy ha restituito un cover NON valido su " << label << "\n";
                        return 2;
                    }
                    appendCsvRow(args.output, n, label, "greedy", sec, (int)cover.size(), "HEURISTIC");
                }

                if (!done.count({label, "ILP"})) {
                    string cmd = "python3 scripts/exact_ilp.py " + graphPath +
                                 " --time-limit " + to_string(args.ilpTimeout) +
                                 " --output " + args.output;
                    int rc = system(cmd.c_str());
                    if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
                        cerr << "ERRORE: exact_ilp.py e' fallito su " << graphPath << "\n";
                        return 3;
                    }
                }
            }
        }
    }

    cout << "Fatto. Risultati in " << args.output << "\n";
    return 0;
}