#!/bin/bash
# Orchestrazione della campagna sperimentale MVC.
# Per ora lancia solo l'esperimento (a) -- B&B vs ILP. Gli altri tre
# (b: qualita' greedy/2-approx, c: scalabilita', d: muro NP-hard) verranno
# aggiunti qui non appena i rispettivi driver saranno pronti.
set -e
cd "$(dirname "$0")"

echo "== Setup =="
if [ -d venv ]; then
    source venv/bin/activate
else
    echo "ATTENZIONE: cartella 'venv' non trovata. Creala con:"
    echo "  python3 -m venv venv && source venv/bin/activate && pip install -r requirements.txt"
    exit 1
fi
pip install -q -r requirements.txt || \
    echo "Attenzione: 'pip install -r requirements.txt' ha dato errore. Proseguo comunque."

echo
echo "== Compilazione =="
# Il Makefile non e' ancora aggiornato alla struttura attuale del progetto:
# compilo direttamente con g++ finche' non lo sara'.
g++ -O2 -std=c++17 -Wall -o mvc src/main.cpp src/approx.cpp src/greedy.cpp src/exact_branch_and_bound.cpp src/brute_force.cpp data/utils.cpp
g++ -O2 -std=c++17 -Wall -o gen_pilot src/gen_pilot.cpp data/utils.cpp
g++ -O2 -std=c++17 -Wall -o experiment_bb_wall src/experiment_bb_wall.cpp src/approx.cpp src/greedy.cpp src/exact_branch_and_bound.cpp src/brute_force.cpp data/utils.cpp
mkdir -p results data

echo
echo "== Esperimento 1: muro esponenziale del B&B =="
./experiment_bb_wall \
    --n0 10 --densities sparse dense --reps 3 --timeout 120 \
    --output results/exp1_bb_wall.csv --graph-dir data/exp1 \
    --skip-existing

echo
echo "== Esperimento 2: muro esponenziale dell'ILP =="
if python3 -c "import gurobipy" 2>/dev/null; then
    ./experiment_ilp_wall \
        --n0 10 --densities sparse dense --reps 3 --timeout $TIMEOUT \
        --output results/exp2_ilp_wall.csv --graph-dir data/exp2 \
        --skip-existing
else
    echo "gurobipy non disponibile in questo venv: salto l'esperimento 2."
    echo "Verifica la licenza Gurobi (vedi Capitolo 2) e rilancia questo script."
fi
echo
echo "== Plot =="
if [ -f results/exp2_bb_wall.csv ]; then
    python3 scripts/plot_bb_wall.py --timeout $TIMEOUT
fi
if [ -f results/exp2_ilp_wall.csv ]; then
    python3 scripts/plot_ilp_wall.py --timeout $TIMEOUT
fi

echo
echo "Fatto. Risultati in results/"