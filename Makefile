# Compilatore e opzioni comuni a tutte le regole.
CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall


TARGETS = mvc gen_pilot experiment_bb_wall experiment_ilp_wall experiment_approx_growth experiment_greedy_growth experiment_quality

# Object file comuni agli eseguibili che usano gli algoritmi (
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