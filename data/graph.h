#ifndef GRAPH_H  // Header Guard: impedisce di includerlo due volte
#define GRAPH_H

#include <vector>
#include <string>

// Definiamo il tipo Graph una volta sola per tutto il progetto
typedef std::vector<std::vector<int>> Graph;

// --- Funzioni di Utilità (staranno in utils.cpp) ---

// --- Algoritmi (staranno nei rispettivi .cpp) ---
int approximation(const Graph& G);

#endif