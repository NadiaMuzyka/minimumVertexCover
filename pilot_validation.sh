#!/bin/bash
set -e
cd "$(dirname "$0")"

SEED=42
mkdir -p results data
rm -f results/pilot.csv

echo "=== test.txt ==="

for n in 10 15 20 25 30 35 40; do

    echo "Generazione grafi di test per n=$n..."
    sparse_deg=$(python3 -c "import math; print(round(math.log2($n), 2))")
    dense_deg=$(python3 -c "print(round($n / 2, 2))")

    for entry in "sparse $sparse_deg" "dense $dense_deg"; do
        tag=${entry%% *}
        deg=${entry#* }
        f="data/pilot_${tag}_n${n}.txt"
        ./gen_pilot "$n" "$deg" "$SEED" "$f" > /dev/null

        ./mvc "$f" bb > /dev/null
        ./mvc "$f" brute > /dev/null
        ./mvc "$f" approx > /dev/null
        ./mvc "$f" greedy > /dev/null
        python3 scripts/exact_ilp.py "$f" --output results/pilot.csv > /dev/null
    done
done
echo "Fatto. Risultati in results/pilot.csv"