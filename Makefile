# Compilatore e opzioni comuni a tutte le regole.
CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall

# Eseguibili attualmente esistenti nel progetto:
# - mvc: demo manuale (un grafo, un algoritmo alla volta)
# - gen_pilot: generatore da riga di comando (Erdos-Renyi by avg degree)
# - experiment_bb_wall: Esperimento 1, muro esponenziale del B&B
# - experiment_ilp_wall: Esperimento 2, muro esponenziale dell'ILP
# Man mano che scriviamo i driver di qualita' e scalabilita', i rispettivi
# target vanno aggiunti qui sotto.
TARGETS = mvc gen_pilot experiment_bb_wall experiment_ilp_wall experiment_approx_growth experiment_greedy_growth experiment_quality

# Object file comuni agli eseguibili che usano gli algoritmi (mvc ed
# experiment_bb_wall): gen_pilot ed experiment_ilp_wall non ne hanno
# bisogno, usano solo il generatore.
OBJS = data/utils.o src/approx.o src/greedy.o src/exact_branch_and_bound.o src/brute_force.o

# Target di default: `make` o `make all` costruisce tutti gli eseguibili.
all: $(TARGETS)

mvc: src/main.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

gen_pilot: src/gen_pilot.o data/utils.o
	$(CXX) $(CXXFLAGS) -o $@ $^

experiment_bb_wall: src/experiment_bb_wall.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

experiment_ilp_wall: src/experiment_ilp_wall.o data/utils.o
	$(CXX) $(CXXFLAGS) -o $@ $^

experiment_approx_growth: src/experiment_approx_growth.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

experiment_greedy_growth: src/experiment_greedy_growth.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

experiment_quality: src/experiment_quality.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regole generiche (pattern rule): "per ottenere X.o, compila X.cpp con -c".
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

data/%.o: data/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# `make clean` cancella tutti gli oggetti compilati e gli eseguibili.
clean:
	rm -f src/*.o data/*.o $(TARGETS)

.PHONY: all clean