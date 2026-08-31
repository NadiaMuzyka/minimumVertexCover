# Compilatore e opzioni comuni a tutte le regole.
# -O2: ottimizzazione (utile visto che misuriamo i tempi di esecuzione)
# -std=c++17: richiesto dagli structured bindings usati nel codice (es. main.cpp)
# -Wall: mostra i warning del compilatore
CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall

# I 5 eseguibili prodotti dal progetto: mvc e' la demo manuale (un grafo,
# un algoritmo alla volta), gli altri sono i runner dei 4 esperimenti.
TARGETS = mvc run_quality run_scale run_nphard run_bb_vs_ilp

# Object file comuni a TUTTI gli eseguibili: gli algoritmi (approx, greedy,
# branch-and-bound) e le utility di lettura/scrittura/generazione grafi.
# Ogni eseguibile li linka tutti, cambia solo il suo main_*.o specifico.
OBJS = data/utils.o src/approx.o src/greedy.o src/exact_branch_and_bound.o src/brute_force.o

# Target di default: `make` o `make all` costruisce tutti e 5 gli eseguibili.
all: $(TARGETS)

# Ogni regola sotto dice "per costruire l'eseguibile X, servono questi .o":
# il main_*.o specifico di quell'esperimento + gli OBJS comuni.
# $@ = nome del target (es. run_quality), $^ = tutte le sue dipendenze.
mvc: src/main.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

run_quality: src/main_quality.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

run_scale: src/main_scale.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

run_nphard: src/main_nphard.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

run_bb_vs_ilp: src/main_bb_vs_ilp.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regole generiche (pattern rule): "per ottenere src/qualcosa.o, compila
# src/qualcosa.cpp con -c (solo compilazione, senza linking)". Grazie a
# queste due regole non serve scrivere una ricetta per ogni singolo .cpp:
# make capisce da solo come trasformare, es., src/greedy.cpp in src/greedy.o.
# $< = il file sorgente (il .cpp), $@ = il file da produrre (il .o).
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

data/%.o: data/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# `make clean` cancella tutti gli oggetti compilati e gli eseguibili, utile
# per ripartire da una build pulita se qualcosa va storto.
clean:
	rm -f src/*.o data/*.o $(TARGETS)

# .PHONY dice a make che "all" e "clean" non sono nomi di file da produrre,
# ma solo etichette di comandi: evita conflitti se esistesse per caso un
# file chiamato "all" o "clean" nella cartella.
.PHONY: all clean
