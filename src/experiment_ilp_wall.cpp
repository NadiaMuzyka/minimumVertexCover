#include <bits/stdc++.h>
using namespace std;
#include <filesystem>
#include <sys/wait.h>
#include "../data/graph.h"
#include "experiment_common.h"

// Esperimento 2: trova il muro esponenziale dell'ILP (Gurobi), separatamente
// per grafi sparsi e densi. Stessa tecnica di experiment_bb_wall: doubling con early stopping 

// Uso (da lanciare dalla root del progetto, con il venv gia' attivato):
//   ./experiment_ilp_wall --n0 10 --densities sparse dense --reps 3
//                          --timeout 120 --confirm-steps 1 --max-doublings 15
//                          --output results/exp2_ilp_wall.csv
//                          --graph-dir data/exp2

struct Args {
    int n0 = 10;                                    // n di partenza del doubling
    vector<string> densities = {"sparse", "dense"}; // livelli da testare, uno per volta
    int reps = 3;                                   // ripetizioni (grafi diversi) per ogni n
    int seedBase = 1000;                             // seed della ripetizione 0; le altre sono seedBase+rep
    double timeout = 120.0;                          // limite di tempo passato a Gurobi (--time-limit), in secondi
    int confirmSteps = 1;                            // raddoppi di conferma dopo il primo TIME_LIMIT
    int maxDoublings = 15;                           // valvola di sicurezza: tetto massimo di raddoppi
    string output = "results/exp2_ilp_wall.csv";     // CSV condiviso, scritto da exact_ilp.py e riletto da lastStatus
    string graphDir = "data/exp2";                   // dove salvare i grafi generati, per riproducibilita'
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

// Delega un singolo grafo a scripts/exact_ilp.py e restituisce true se lo
// stato risultante e' TIME_LIMIT.
static bool runTrial(int n, const string& density, int seed, double avgDeg, const Args& args) {

    string label = "n" + to_string(n) + "_" + density + "_s" + to_string(seed);
    string graphPath = args.graphDir + "/" + label + ".txt";

    Graph G = generateErdosRenyiByAvgDegree(n, avgDeg, (unsigned)seed);
    writeGraphToFile(G, graphPath);

    string cmd = "python3 scripts/exact_ilp.py " + graphPath +
                 " --time-limit " + to_string(args.timeout) +
                 " --output " + args.output;
    int rc = system(cmd.c_str());
    if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
        cerr << "ERRORE: exact_ilp.py e' fallito su " << graphPath << "\n";
        exit(3);
    }

    // Lo script scrive il proprio esito direttamente nel CSV condiviso (non
    // lo stampa in un formato pensato per essere riletto qui): per sapere se
    // questo trial e' andato in TIME_LIMIT, si rilegge l'ultima riga che lo
    // riguarda invece di catturarne l'output.
    string status = lastStatus(args.output, label, "ILP");
    cout << "  n=" << n << " seed=" << seed << ": " << status << endl;
    return status == "TIME_LIMIT";
}

int main(int argc, char* argv[]) {
    Args args = parseArgs(argc, argv);
    filesystem::create_directories(args.graphDir);
    filesystem::create_directories(filesystem::path(args.output).parent_path());

    for (const string& density : args.densities) {
        cout << "=== Densita': " << density << " ===" << endl;
        int n = args.n0;
        // wallStep = -1: muro non ancora trovato.
        // wallStep >= 0: muro trovato, conta quanti raddoppi di conferma sono
        // gia' passati da quel momento.
        int wallStep = -1;

        for (int step = 0; step < args.maxDoublings; step++) {
            double avgDeg = (density == "sparse") ? sparseAvgDegree(n) : denseAvgDegree(n);
            bool anyTimeout = false;

            // reps grafi diversi allo stesso n: basta un TIME_LIMIT su una sola
            // ripetizione per considerare n "a rischio" e far scattare la fase
            // di conferma.
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
                wallStep++;
            }
            // Ci si ferma un passo di raddoppio dopo il primo timeout (non subito),
            // per non scambiare un timeout isolato per il muro vero e proprio.
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