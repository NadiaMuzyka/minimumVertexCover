#ifndef EXPERIMENT_COMMON_H
#define EXPERIMENT_COMMON_H

#include <bits/stdc++.h>
#include <filesystem>

// Funzioni comuni a tutti i driver sperimentali: formule di
// densita', parsing di liste da riga di comando, lettura/scrittura del CSV condiviso.

inline double sparseAvgDegree(int n) {
    return std::round(2.0 * std::log((double)n) * 100.0) / 100.0;
}
inline double denseAvgDegree(int n) {
    return std::round((n / 2.0) * 100.0) / 100.0;
}

inline std::vector<std::string> collectValues(int& i, int argc, char* argv[]) {
    std::vector<std::string> vals;
    while (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) {
        vals.push_back(argv[++i]);
    }
    return vals;
}

inline void appendCsvRow(const std::string& path, int n, const std::string& label,
                          const std::string& algo, double sec, int cardinality,
                          const std::string& status) {
    bool writeHeader = !std::filesystem::exists(path);
    std::ofstream csv(path, std::ios::app);
    if (writeHeader)
        csv << "N_Nodi,Grafo,Algoritmo,CPU_Time_sec,Cardinalita,Status\n";
    csv << n << "," << label << "," << algo << "," << std::fixed << std::setprecision(6) << sec
        << "," << cardinality << "," << status << "\n";
}

inline long long estimatedEdges(int n, double avgDegree) {
    return (long long)n * (long long)std::llround(avgDegree) / 2;
}

inline bool isStable(const std::vector<double>& ratios, int window, double tolerance) {
    if ((int)ratios.size() < window) return false;
    double mn = 1e18, mx = -1e18;
    for (int i = (int)ratios.size() - window; i < (int)ratios.size(); i++) {
        mn = std::min(mn, ratios[i]);
        mx = std::max(mx, ratios[i]);
    }
    return (mx - mn) / mn <= tolerance;
}

inline std::string lastStatus(const std::string& path, const std::string& label,
                               const std::string& algo) {
    std::ifstream fin(path);
    std::string line, status;
    std::getline(fin, line);
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string nNodi, grafo, alg, time, card, st;
        std::getline(ss, nNodi, ',');
        std::getline(ss, grafo, ',');
        std::getline(ss, alg, ',');
        std::getline(ss, time, ',');
        std::getline(ss, card, ',');
        std::getline(ss, st, ',');
        if (grafo == label && alg == algo) status = st;
    }
    return status;
}

#endif