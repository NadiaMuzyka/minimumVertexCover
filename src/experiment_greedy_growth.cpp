#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include "../data/graph.h"

// Esperimento 3b: caratterizza l'andamento del greedy al crescere di n.
// Stessa logica di experiment_approx_growth (doubling con early stopping
// sulla stabilizzazione del rapporto, DOE Guideline 2.7), ma qui il
// risultato atteso e' diverso: il greedy ha complessita' O(|V|^2+|E|)
// (Cap. 1, confermato dal codice in src/greedy.cpp: ad ogni passo scandisce
// tutti gli n vertici per trovare quello di grado massimo) -- ci si aspetta
// quindi un rapporto vicino a 4 (quadratico), non vicino a 2 come per il
// 2-approssimato, e su ENTRAMBE le densita' (il termine V^2 domina anche
// sullo sparso, dove |E| resta piccolo).
//
// A differenza del generatore, qui il costo O(n^2) e' nell'algoritmo
// stesso: non c'e' equivalente della "versione veloce" da applicare.
// --max-n di default e' piu' basso che nell'esperimento dell'approx per
// questo motivo.
//
// Uso:
//   ./experiment_greedy_growth --n0 1000 --densities sparse dense --reps 7
//                               --stability-window 3 --tolerance 0.25
//                               --max-doublings 12 --max-edges 70000000
//                               --max-n 100000
//                               --output results/exp3_greedy_growth.csv
//                               [--skip-existing]

struct Args {
    int n0 = 1000;
    vector<string> densities = {"sparse", "dense"};
    int reps = 7;
    int seedBase = 1000;
    int stabilityWindow = 3;
    double tolerance = 0.25;
    int maxDoublings = 12;
    long long maxEdges = 70000000;
    int maxN = 100000;
    string output = "results/exp3_greedy_growth.csv";
    bool skipExisting = false;
};

static double sparseAvgDegree(int n) { return round(2.0 * log((double)n) * 100.0) / 100.0; }
static double denseAvgDegree(int n) { return round((n / 2.0) * 100.0) / 100.0; }

static long long estimatedEdges(int n, double avgDegree) {
    return (long long)n * (long long)llround(avgDegree) / 2;
}

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
        else if (flag == "--stability-window") a.stabilityWindow = stoi(argv[++i]);
        else if (flag == "--tolerance") a.tolerance = stod(argv[++i]);
        else if (flag == "--max-doublings") a.maxDoublings = stoi(argv[++i]);
        else if (flag == "--max-edges") a.maxEdges = stoll(argv[++i]);
        else if (flag == "--max-n") a.maxN = stoi(argv[++i]);
        else if (flag == "--output") a.output = argv[++i];
        else if (flag == "--skip-existing") a.skipExisting = true;
        else { cerr << "Argomento sconosciuto: " << flag << "\n"; exit(1); }
    }
    return a;
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

static bool loadExistingStep(const string& path, int n, const string& density, int reps,
                              double& meanTime) {
    ifstream fin(path);
    if (!fin) return false;
    string prefix = "n" + to_string(n) + "_" + density + "_s";
    string line;
    getline(fin, line);
    int count = 0;
    double sum = 0;
    while (getline(fin, line)) {
        stringstream ss(line);
        string nNodi, grafo, algo, time, card, status;
        getline(ss, nNodi, ','); getline(ss, grafo, ','); getline(ss, algo, ',');
        getline(ss, time, ','); getline(ss, card, ','); getline(ss, status, ',');
        if (grafo.rfind(prefix, 0) != 0) continue;
        if (algo == "greedy") { sum += stod(time); count++; }
    }
    if (count >= reps) {
        meanTime = sum / count;
        return true;
    }
    return false;
}

static bool isStable(const vector<double>& ratios, int window, double tolerance) {
    if ((int)ratios.size() < window) return false;
    double mn = 1e18, mx = -1e18;
    for (int i = (int)ratios.size() - window; i < (int)ratios.size(); i++) {
        mn = min(mn, ratios[i]);
        mx = max(mx, ratios[i]);
    }
    return (mx - mn) / mn <= tolerance;
}

int main(int argc, char* argv[]) {
    Args args = parseArgs(argc, argv);
    filesystem::create_directories(filesystem::path(args.output).parent_path());

    for (const string& density : args.densities) {
        cout << "=== Densita': " << density << " ===" << endl;
        int n = args.n0;
        vector<double> times;
        vector<double> ratios;

        for (int step = 0; step < args.maxDoublings; step++) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);
            long long edges = estimatedEdges(n, avgDeg);
            if (edges > args.maxEdges) {
                cout << "Fermato per " << density << ": a n=" << n << " ci si aspettano ~"
                     << edges << " archi, sopra --max-edges (" << args.maxEdges << ")" << endl;
                break;
            }
            if (n > args.maxN) {
                cout << "Fermato per " << density << ": n=" << n << " sopra --max-n ("
                     << args.maxN << ") -- il greedy stesso e' O(n^2), oltre questo "
                     << "tetto ogni passo costa sensibilmente di piu'" << endl;
                break;
            }

            double meanTime;
            bool cached = args.skipExisting &&
                          loadExistingStep(args.output, n, density, args.reps, meanTime);

            if (!cached) {
                double sum = 0;
                for (int rep = 0; rep < args.reps; rep++) {
                    int seed = args.seedBase + rep;
                    string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);

                    Graph G = generateErdosRenyiFastByAvgDegree(n, avgDeg, (unsigned)seed);

                    auto t0 = chrono::steady_clock::now();
                    vector<int> cover = greedyVertexCover(G);
                    auto t1 = chrono::steady_clock::now();
                    double sec = chrono::duration<double>(t1 - t0).count();

                    if (!isValidCover(G, cover)) {
                        cerr << "ERRORE: cover NON valido su " << label << "\n";
                        return 2;
                    }

                    appendCsvRow(args.output, n, label, "greedy", sec, (int)cover.size(), "HEURISTIC");
                    sum += sec;
                }
                meanTime = sum / args.reps;
            }

            times.push_back(meanTime);

            if (times.size() >= 2) {
                double r = meanTime / times[times.size() - 2];
                ratios.push_back(r);
                cout << "  n=" << n << ": tempo medio=" << fixed << setprecision(6) << meanTime
                     << "s (rapporto=" << setprecision(3) << r << ")" << endl;
            } else {
                cout << "  n=" << n << ": tempo medio=" << fixed << setprecision(6) << meanTime << "s" << endl;
            }

            if (isStable(ratios, args.stabilityWindow, args.tolerance)) {
                cout << ">>> Andamento stabilizzato per " << density << " a n=" << n << endl;
                break;
            }

            n *= 2;
        }
    }

    cout << "Fatto. Risultati in " << args.output << "\n";
    return 0;
}