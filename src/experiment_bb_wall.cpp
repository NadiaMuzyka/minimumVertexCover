#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include "../data/graph.h"
#include "experiment_common.h"

// Esperimento 1: trova il muro esponenziale del B&B, separatamente per
// grafi sparsi e densi (i due muri sono troppo distanti per condividere lo
// stesso range). 
//
// Uso:
//   ./experiment_bb_wall --n0 10 --densities sparse dense --reps 3
//                         --timeout 120 --confirm-steps 1 --max-doublings 15
//                         --output results/exp1_bb_wall.csv
//                         --graph-dir data/exp1 [--skip-existing]

struct Args {
    int n0 = 10;                                    // n di partenza del doubling
    vector<string> densities = {"sparse", "dense"}; // livelli da testare, uno per volta
    int reps = 3;                                   // ripetizioni (grafi diversi) per ogni n
    int seedBase = 1000;                             // seed della ripetizione 0; le altre sono seedBase+rep
    double timeout = 120.0;                          // limite di tempo passato al B&B, in secondi
    int confirmSteps = 1;                            // raddoppi di conferma dopo il primo TIME_LIMIT
    int maxDoublings = 15;                           // valvola di sicurezza: tetto massimo di raddoppi
    string output = "results/exp1_bb_wall.csv";      // CSV condiviso su cui si appende una riga per trial
    string graphDir = "data/exp1";                   // dove salvare i grafi generati, per riproducibilita'
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
        else if (flag == "--timeout") a.timeout = stod(argv[++i]);
        else if (flag == "--confirm-steps") a.confirmSteps = stoi(argv[++i]);
        else if (flag == "--max-doublings") a.maxDoublings = stoi(argv[++i]);
        else if (flag == "--output") a.output = argv[++i];
        else if (flag == "--graph-dir") a.graphDir = argv[++i];
        else { cerr << "Argomento sconosciuto: " << flag << "\n"; exit(1); }
    }
    return a;
}

// Esegue un singolo trial (un grafo, un algoritmo, un n) e ne registra l'esito.
// Ritorna true se il trial ha dato TIME_LIMIT, false altrimenti: il chiamante usa
// questo booleano per decidere quando e' stato trovato il muro.
static bool runTrial(int n, const string& density, int seed, double avgDeg, const Args& args) {
    // Etichetta univoca del design point (n, densita', seed): identifica la riga
    // nel CSV e il file su disco del grafo usato per generarla.
    string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);
    string graphPath = args.graphDir + "/" + label + ".txt";

    Graph G = generateErdosRenyiByAvgDegree(n, avgDeg, (unsigned)seed);

    auto t0 = chrono::steady_clock::now();
    ExactResult r = exactVertexCoverStats(G, args.timeout);
    auto t1 = chrono::steady_clock::now();
    double sec = chrono::duration<double>(t1 - t0).count();

    // Controllo di sanita': anche in caso di TIME_LIMIT, la soluzione parziale
    // restituita deve comunque essere un cover valido.
    if (!isValidCover(G, r.cover)) {
        cerr << "ERRORE: B&B ha restituito un cover NON valido su " << graphPath << "\n";
        exit(2);
    }
    string status = r.timedOut ? "TIME_LIMIT" : "OPTIMAL";
    appendCsvRow(args.output, n, label, "bb", sec, (int)r.cover.size(), status);
    cout << "  n=" << n << " seed=" << seed << ": " << fixed << setprecision(3)
         << sec << "s (" << status << ")" << endl;
    return r.timedOut;
}

int main(int argc, char* argv[]) {
    Args args = parseArgs(argc, argv);
    filesystem::create_directories(args.graphDir);
    filesystem::create_directories(filesystem::path(args.output).parent_path());

    // Sparso e denso vengono trattati come due esperimenti indipendenti in sequenza
    for (const string& density : args.densities) {
        cout << "=== Densita': " << density << " ===" << endl;
        int n = args.n0;

        // wallStep = -1: muro non ancora trovato.
        // wallStep >= 0: muro trovato, conta quanti raddoppi di conferma sono
        // gia' passati da quel momento 
        int wallStep = -1;

        for (int step = 0; step < args.maxDoublings; step++) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);
            bool anyTimeout = false;

            // reps grafi diversi allo stesso n, con seed distinti ma deterministici:
            // basta un TIME_LIMIT su una sola ripetizione per considerare n "a rischio",
            // cosi' da non perdere un muro che si manifesta solo su certe istanze
            for (int rep = 0; rep < args.reps; rep++) {
                int seed = args.seedBase + rep;
                if (runTrial(n, density, seed, avgDeg, args)) anyTimeout = true;
            }

            if (anyTimeout && wallStep == -1) {
                // Primo timeout osservato a questo n: segna l'inizio della fase
                // di conferma, ma non ci si ferma subito.
                wallStep = 0;
                cout << ">>> Muro trovato per " << density << " a n=" << n << endl;
            } else if (wallStep >= 0) {
                // Gia' oltre il primo timeout: si e' in fase di conferma,
                // a prescindere dal fatto che anche questo passo abbia avuto timeout.
                wallStep++;
            }
            // Ci si ferma un passo di raddoppio dopo il primo timeout (non subito),
            // per non scambiare un timeout isolato per il muro vero e proprio.
            if (wallStep >= args.confirmSteps) break;

            n *= 2;
        }
    }

    cout << "Fatto. Risultati in " << args.output << "\n";
    return 0;
}