# Minimum Vertex Cover — Algorithm Engineering

Tesina per il corso di Ingegneria degli Algoritmi, Università degli Studi dell'Aquila (A.A. 2025/2026).

Implementazione e valutazione sperimentale di quattro approcci al problema del Minimum Vertex Cover: un algoritmo esatto per forza bruta, un Branch-and-Bound, una formulazione ILP risolta con Gurobi, e due euristiche polinomiali (2-approssimato, greedy). L'obiettivo è caratterizzare empiricamente i limiti pratici di ciascun metodo e il trade-off tra tempo di esecuzione e qualità della soluzione, non solo dimostrarne la correttezza teorica.

## Struttura del repository
```bash
src/ algoritmi (approx, greedy, forza bruta, B&B) e driver sperimentali
data/ tipo Graph, generatori di grafi, parsing/scrittura file
scripts/ script Python: ILP via Gurobi, plotting dei risultati
results/ CSV e grafici prodotti dagli esperimenti
requirements.txt dipendenze Python (venv)
Makefile build di tutti gli eseguibili C++
run_experiments.sh orchestrazione della campagna sperimentale completa
pilot_validation.sh script del pilot 
```

## Requisiti

- **C++17** (g++)
- **Python 3** con un virtualenv: `python3 -m venv venv && . venv/bin/activate && pip install -r requirements.txt`
- **Licenza Gurobi**: la formulazione ILP (esperimenti 2 e 4) richiede `gurobipy`. La licenza "size-limited" gratuita basta per grafi piccoli, ma ha un tetto di 2000 variabili/vincoli — per i range usati in questo progetto serve una licenza accademica (gratuita per studenti universitari, vedi [gurobi.com](https://www.gurobi.com)).

## Pipeline sperimentale

Tutti gli esperimenti seguono la stessa metodologia: doubling experiment per scegliere le dimensioni da testare, timeout dichiarato esplicitamente per i metodi esatti, più ripetizioni per punto con seed diversi.

| # | Esperimento | Obiettivo | Comando | Output |
|---|---|---|---|---|
| Pilot | Validazione | I tre metodi esatti concordano? | `./pilot_validation.sh` | `results/pilot*.png` |
| 1 | Muro B&B | Fino a che $n$ il Branch-and-Bound resta praticabile | `./experiment_bb_wall --n0 10 --densities sparse dense --reps 3 --timeout 120` | `exp1_bb_wall.csv/.png` |
| 2 | Muro ILP | Fino a che $n$ l'ILP (Gurobi) resta praticabile | `./experiment_ilp_wall --n0 10 --densities sparse dense --reps 3 --timeout 120` | `exp2_ilp_wall.csv/.png` |
| 3a | Crescita 2-approx | Conferma empirica di $O(\lvert V\rvert+\lvert E\rvert)$ | `./experiment_approx_growth --n0 1000 --densities sparse dense --reps 7` | `exp3_approx_*.csv/.png` |
| 3b | Crescita greedy | Conferma empirica di $O(\lvert V\rvert^2+\lvert E\rvert)$ | `./experiment_greedy_growth --n0 1000 --densities sparse dense --reps 7` | `exp3_greedy_*.csv/.png` |
| 4 | Qualità | Quanto si discostano le euristiche dall'ottimo (ILP) | `./experiment_quality --reps 5 --ilp-timeout 120` | `exp_quality_*.csv/.png` |

