#!/bin/bash
# Orchestrazione della campagna sperimentale MVC.
# Esperimenti 1 (muro B&B), 2 (muro ILP), 3a (crescita 2-approx),
# 3b (crescita greedy), 4 (qualita' vs ILP) -- con i relativi plot.
set -e
cd "$(dirname "$0")"

LOG_FILE="run_experiments_$(date +%Y%m%d_%H%M%S).log"
exec > >(tee "$LOG_FILE") 2>&1
echo "Log salvato in: $LOG_FILE"

TIMEOUT=120

echo "== Setup =="
if [ -d venv ]; then
    . venv/bin/activate
else
    echo "ATTENZIONE: cartella 'venv' non trovata. Creala con:"
    echo "  python3 -m venv venv && . venv/bin/activate && pip install -r requirements.txt"
    exit 1
fi
pip install -q -r requirements.txt || \
    echo "Attenzione: 'pip install -r requirements.txt' ha dato errore. Proseguo comunque."

echo
echo "== Compilazione =="
make clean
make all
mkdir -p results data

echo
echo "== Esperimento 1: muro esponenziale del B&B =="
./experiment_bb_wall \
    --n0 10 --densities sparse dense --reps 3 --timeout $TIMEOUT \
    --output results/exp1_bb_wall.csv --graph-dir data/exp1 \
    --skip-existing

echo
echo "== Esperimento 2: muro esponenziale dell'ILP =="
# experiment_ilp_wall (ed experiment_quality piu' sotto) invocano
# "python3 scripts/exact_ilp.py" internamente via system(): serve il venv
# gia' attivato (fatto sopra), altrimenti quel "python3" risolverebbe al
# Python di sistema, senza gurobipy.
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
echo "== Esperimento 3a: andamento del 2-approssimato =="
./experiment_approx_growth \
    --n0 1000 --densities sparse dense --reps 7 \
    --stability-window 3 --tolerance 0.25 --max-doublings 12 \
    --max-edges 70000000 --max-n 200000 \
    --output results/exp3_approx_growth.csv \
    --skip-existing

echo
echo "== Esperimento 3b: andamento del greedy =="
./experiment_greedy_growth \
    --n0 1000 --densities sparse dense --reps 7 \
    --stability-window 3 --tolerance 0.25 --max-doublings 12 \
    --max-edges 70000000 --max-n 100000 \
    --output results/exp3_greedy_growth.csv \
    --skip-existing

echo
echo "== Esperimento 4: qualita' (approx/greedy vs ILP) =="
if python3 -c "import gurobipy" 2>/dev/null; then
    ./experiment_quality \
        --reps 5 --ilp-timeout $TIMEOUT \
        --output results/exp_quality.csv --graph-dir data/exp_quality \
        --skip-existing
else
    echo "gurobipy non disponibile in questo venv: salto l'esperimento 4."
    echo "Verifica la licenza Gurobi (vedi Capitolo 2) e rilancia questo script."
fi

echo
echo "== Plot =="
if [ -f results/exp1_bb_wall.csv ]; then
    python3 scripts/plot_bb_wall.py --timeout $TIMEOUT
fi
if [ -f results/exp2_ilp_wall.csv ]; then
    python3 scripts/plot_ilp_wall.py --timeout $TIMEOUT
fi
if [ -f results/exp3_approx_growth.csv ]; then
    python3 scripts/plot_approx_growth.py
fi
if [ -f results/exp3_greedy_growth.csv ]; then
    python3 scripts/plot_greedy_growth.py
fi
if [ -f results/exp_quality.csv ]; then
    python3 scripts/plot_quality_final.py
fi

echo
echo "Fatto. Risultati (CSV + PNG) in results/"