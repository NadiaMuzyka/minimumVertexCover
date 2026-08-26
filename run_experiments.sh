#!/bin/bash
# Orchestrazione della sperimentazione MVC: compila, esegue i 4 esperimenti
# e genera i relativi plot. La parte ILP/Gurobi dell'esperimento (a) viene
# eseguita solo se gurobipy risulta installato e licenziato (Step 0 del
# piano); altrimenti viene saltata con un avviso, senza interrompere il resto.
set -e
#Posiziona la working directory nella cartella dello script, così da poterlo lanciare da qualsiasi path.
cd "$(dirname "$0")"

VENV_PY=venv/bin/python
VENV_PIP=venv/bin/pip

#Installazione delle dipendenze (se non già presenti) 
echo "== Setup =="
$VENV_PIP install -q -r requirements.txt || \
    echo "Attenzione: 'pip install -r requirements.txt' ha dato errore (probabile gurobipy senza licenza attiva). Proseguo comunque."

# Compilazione e creazione cartella risultati
make clean
make all
mkdir -p results

# Esecuzione esperimenti
echo
echo "== Esperimento (b): Greedy vs 2-Approx =="
./run_quality

echo
echo "== Esperimento (c): Scalabilita' =="
./run_scale

echo
echo "== Esperimento (d): NP-hardness wall =="
./run_nphard

echo
echo "== Esperimento (a): Branch-and-Bound (parte C++) =="
./run_bb_vs_ilp

#controllo se gurobipy è disponibile, altrimenti salto l'esperimento ILP/Gurobi
echo
echo "== Esperimento (a): ILP/Gurobi =="
if $VENV_PY -c "import gurobipy" 2>/dev/null; then
    rm -f results/ilp.csv
    for f in results/tmp_graph_*.txt data/small_sparse.txt data/small_dense.txt; do
        $VENV_PY scripts/exact_ilp.py "$f" --output results/ilp.csv
    done
else
    echo "gurobipy non disponibile: salto l'esperimento ILP."
    echo "Completa lo Step 0 del piano (licenza Gurobi + 'pip install gurobipy') e rilancia questo script, oppure esegui manualmente scripts/exact_ilp.py sui file in results/tmp_graph_*.txt."
fi

# Generazione plot
echo
echo "== Plot =="
$VENV_PY scripts/plot_quality.py
$VENV_PY scripts/plot_scale.py
$VENV_PY scripts/plot_nphard.py
if [ -f results/ilp.csv ]; then
    $VENV_PY scripts/plot_bb_vs_ilp.py
fi

echo
echo "Esperimenti completati. Risultati (CSV + PNG) in results/"
