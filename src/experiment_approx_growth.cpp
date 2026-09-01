#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include "../data/graph.h"

// Esperimento 3: caratterizza l'andamento del 2-approssimato al crescere di
// n. Non c'e' un evento su cui fermarsi (e' polinomiale per costruzione):
// il criterio di stop e' la STABILIZZAZIONE del rapporto tempo(2n)/tempo(n)
// (DOE, "Doubling Experiment: More Patterns", p.31/68) -- ci si ferma
// quando il rapporto resta entro una tolleranza per un certo numero di
// raddoppi consecutivi, segno che la forma della crescita e' riconoscibile.
//
// Valvola di sicurezza sugli archi: sui grafi densi (m ~ n^2/4) pochi
// raddoppi bastano per generare centinaia di milioni di archi -- il ciclo
// si ferma anche se il prossimo passo supererebbe --max-edges,
// indipendentemente dalla stabilita' del rapporto.
//
// Uso:
//   ./experiment_approx_growth --n0 1000 --densities sparse dense --reps 3
//                               --stability-window 3 --tolerance 0.2
//                               --max-doublings 12 --max-edges 20000000
//                               --output results/exp3_approx_growth.csv
//                               --graph-dir data/exp3 [--skip-existing]

struct Args {
    int n0 = 1000;
    vector<string> densities = {"sparse", "dense"};
    int reps = 3;
    int seedBase = 1000;
    int stabilityWindow = 3;
    double tolerance = 0.2;
    int maxDoublings = 12;
    long long maxEdges = 40000000;
    int maxN = 200000;
    string output = "results/exp3_approx_growth.csv";
    string graphDir = "data/exp3";
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
        else if (flag == "--graph-dir") a.graphDir = argv[++i];
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

// Controlla se questo passo (n, densita') e' gia' completo nel CSV -- tutte
// le repliche gia' presenti -- e in tal caso ne ricalcola la media senza
// rieseguire nulla. Granularita' per passo intero, per evitare righe
// duplicate su un resume.
static bool loadExistingStep(const string& path, int n, const string& density, int reps,
                              double& meanTime) {
    ifstream fin(path);
    if (!fin) return false;
    string prefix = "n" + to_string(n) + "_" + density + "_s";
    string line;
    getline(fin, line); // intestazione
    int count = 0;
    double sum = 0;
    while (getline(fin, line)) {
        stringstream ss(line);
        string nNodi, grafo, algo, time, card, status;
        getline(ss, nNodi, ','); getline(ss, grafo, ','); getline(ss, algo, ',');
        getline(ss, time, ','); getline(ss, card, ','); getline(ss, status, ',');
        if (grafo.rfind(prefix, 0) != 0) continue;
        if (algo == "approx") { sum += stod(time); count++; }
    }
    if (count >= reps) {
        meanTime = sum / count;
        return true;
    }
    return false;
}

// Stabile se, nella finestra delle ultime `window` misure, il rapporto
// massimo/minimo resta entro la tolleranza relativa data.
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
                     << args.maxN << ") -- il generatore e' O(n^2) indipendentemente "
                     << "dalla densita', oltre questo tetto diventa troppo lento" << endl;
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
                    // Niente writeGraphToFile qui: a differenza degli esperimenti con
                    // l'ILP, questo misura solo il tempo in memoria e non deve mai
                    // rileggere il grafo da un altro processo -- scriverlo su disco
                    // sarebbe solo un costo aggiuntivo, pesante sui grafi densi grandi.

                    auto t0 = chrono::steady_clock::now();
                    vector<int> cover = approxVertexCover(G);
                    auto t1 = chrono::steady_clock::now();
                    double sec = chrono::duration<double>(t1 - t0).count();

                    if (!isValidCover(G, cover)) {
                        cerr << "ERRORE: cover NON valido su " << label << "\n";
                        return 2;
                    }

                    appendCsvRow(args.output, n, label, "approx", sec, (int)cover.size(), "HEURISTIC");
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