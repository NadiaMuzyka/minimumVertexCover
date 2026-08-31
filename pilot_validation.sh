#!/bin/bash
set -e
cd "$(dirname "$0")"

SEED=42
mkdir -p results data
rm -f results/pilot.csv

echo "=== test.txt ==="
./mvc data/test.txt brute > /dev/null
./mvc data/test.txt bb > /dev/null
python3 scripts/exact_ilp.py data/test.txt --output results/pilot.csv > /dev/null

for n in 10 20 30 40; do
    sparse_deg=$(python3 -c "import math; print(round(math.log2($n), 2))")
    dense_deg=$(python3 -c "print(round($n / 2, 2))")

    for entry in "sparse $sparse_deg" "dense $dense_deg"; do
        tag=${entry%% *}
        deg=${entry#* }
        f="data/pilot_${tag}_n${n}.txt"
        ./gen_pilot "$n" "$deg" "$SEED" "$f" > /dev/null

        if [ "$n" -le 60 ]; then
            echo "=== $f: forza bruta (puo' richiedere ~20s a n=30) ==="
            ./mvc "$f" brute > /dev/null
        else
            echo "=== $f: forza bruta SALTATA (n=$n, non praticabile) ==="
        fi
        ./mvc "$f" bb > /dev/null
        python3 scripts/exact_ilp.py "$f" --output results/pilot.csv > /dev/null
    done
done
echo "Fatto. Risultati in results/pilot.csv"