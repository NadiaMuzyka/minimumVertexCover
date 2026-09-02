#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include "../data/graph.h"
#include "experiment_common.h"

// Esperimento 3a: caratterizza l'andamento del 2-approssimato al crescere di
// n. Non c'e' un evento su cui fermarsi (e' polinomiale per costruzione):
// il criterio di stop e' la STABILIZZAZIONE del rapporto tempo(2n)/tempo(n)
// ci si ferma quando il rapporto resta entro una tolleranza per un certo numero di
// raddoppi consecutivi, segno che la forma della crescita e' riconoscibile.

// Uso:
//   ./experiment_approx_growth --n0 1000 --densities sparse dense --reps 3
//                               --stability-window 3 --tolerance 0.2
//                               --max-doublings 12 --max-edges 40000000
//                               --output results/exp3_approx_growth.csv

struct Args {
    int n0 = 1000;                                   // n di partenza del doubling
    vector<string> densities = {"sparse", "dense"}; // livelli da testare, uno per volta
    int reps = 3;                                    // ripetizioni (grafi diversi) per ogni n
    int seedBase = 1000;                             // seed della ripetizione 0; le altre sono seedBase+rep
    int stabilityWindow = 3;                         // quanti rapporti consecutivi devono restare stabili
    double tolerance = 0.2;                          // tolleranza relativa (0.2 = 20%) per considerarli "stabili"
    int maxDoublings = 12;                           // valvola di sicurezza: tetto massimo di raddoppi
    long long maxEdges = 40000000;                   // valvola di sicurezza: tetto sugli archi attesi (grafi densi)
    int maxN = 200000;                                // valvola di sicurezza: tetto su n (generatore O(n^2))
    string output = "results/exp3_approx_growth.csv";
};

// Legge i flag da riga di comando sovrascrivendo i default di Args.
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
        else { cerr << "Argomento sconosciuto: " << flag << "\n"; exit(1); }
    }
    return a;
}

// Esegue le repliche di un singolo passo (n, densita') e restituisce il
// tempo medio del 2-approssimato.
static double runStep(int n, const string& density, double avgDeg, const Args& args) {
    double sum = 0;
    for (int rep = 0; rep < args.reps; rep++) {
        int seed = args.seedBase + rep;
        string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);

        Graph G = generateErdosRenyiFastByAvgDegree(n, avgDeg, (unsigned)seed);

        auto t0 = chrono::steady_clock::now();
        vector<int> cover = approxVertexCover(G);
        auto t1 = chrono::steady_clock::now();
        double sec = chrono::duration<double>(t1 - t0).count();

        // Controllo di sanita': il 2-approssimato deve restituire sempre un
        // cover valido (a differenza della cardinalita', sulla cui ottimalita'
        // non da' garanzie).
        if (!isValidCover(G, cover)) {
            cerr << "ERRORE: cover NON valido su " << label << "\n";
            exit(2);
        }

        appendCsvRow(args.output, n, label, "approx", sec, (int)cover.size(), "HEURISTIC");
        sum += sec;
    }
    return sum / args.reps;
}

int main(int argc, char* argv[]) {
    Args args = parseArgs(argc, argv);
    filesystem::create_directories(filesystem::path(args.output).parent_path());

    // Sparso e denso vengono trattati come due esperimenti indipendenti in sequenza
    // hanno complessita' attese diverse 
    for (const string& density : args.densities) {
        cout << "=== Densita': " << density << " ===" << endl;
        int n = args.n0;
        vector<double> times;  // tempo medio ad ogni n testato, in ordine di raddoppio
        vector<double> ratios; // rapporto times[k]/times[k-1] ad ogni passo, usato per la stabilizzazione

        for (int step = 0; step < args.maxDoublings; step++) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);

            // Le due valvole di sicurezza: qui non c'e' un timeout su cui fermarsi
            // (l'algoritmo e' polinomiale), quindi il rischio e' continuare a
            // raddoppiare ben oltre il ragionevole. Si stima il numero di archi
            // attesi prima di generare il grafo, per non pagarne il costo invano.
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

            double meanTime = runStep(n, density, avgDeg, args);
            times.push_back(meanTime);

            // Il rapporto tempo(n)/tempo(n/2) e' calcolabile solo dal secondo
            // passo in poi (serve un termine di paragone precedente).
            if (times.size() >= 2) {
                double r = meanTime / times[times.size() - 2];
                ratios.push_back(r);
                cout << "  n=" << n << ": tempo medio=" << fixed << setprecision(6) << meanTime
                     << "s (rapporto=" << setprecision(3) << r << ")" << endl;
            } else {
                cout << "  n=" << n << ": tempo medio=" << fixed << setprecision(6) << meanTime << "s" << endl;
            }

            // Criterio di stop "vero": gli ultimi stabilityWindow rapporti restano
            // entro tolerance l'uno dall'altro, segno che la forma della crescita
            // (vicina a 2 = lineare, a 4 = quadratica, ...) e' ormai riconoscibile.
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