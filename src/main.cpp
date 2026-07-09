#include <bits/stdc++.h>
using namespace std;
#include "../data/graph.h"

vector<int> approxVertexCover(const Graph& G);

int main() {
    const string path = "../data/test.txt";

    ifstream fin(path);
    if (!fin) {
        cerr << "Cannot open file: " << path << "\n";
        return 1;
    }

    // Remap arbitrary node IDs to 0..n-1
    unordered_map<int,int> id2idx;
    vector<int> idx2id;
    vector<pair<int,int>> edges;
    string line;

    auto getIdx = [&](int raw) -> int {
        auto [it, inserted] = id2idx.emplace(raw, (int)id2idx.size());
        if (inserted) idx2id.push_back(raw);
        return it->second;
    };

    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int u, v;
        if (!(ss >> u >> v)) continue;
        int a = getIdx(u), b = getIdx(v);
        if (a != b)
            edges.emplace_back(a, b);
    }

    int n = (int)id2idx.size();

    Graph G(n);
    for (auto [a, b] : edges) {
        G[a].push_back(b);
        G[b].push_back(a);
    }

    cout << "Nodes: " << n << "  Edges: " << edges.size() << "\n";

    auto t0 = chrono::steady_clock::now();
    vector<int> cover = approxVertexCover(G);
    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, milli>(t1 - t0).count();

    cout << "Vertex cover (" << cover.size() << " nodi): ";
    for (int idx : cover) cout << idx2id[idx] << " ";
    cout << "\n";
    cout << "Cardinalita': " << cover.size() << "\n";
    cout << "Tempo: " << fixed << setprecision(3) << ms << " ms\n";

    ofstream fout("../results/approx_cover.txt");
    fout << cover.size() << "\n";
    for (size_t i = 0; i < cover.size(); ++i)
        fout << idx2id[cover[i]] << (i + 1 < cover.size() ? ' ' : '\n');

    return 0;
}
