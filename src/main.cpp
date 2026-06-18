#include <bits/stdc++.h>
using namespace std;

vector<int> approxVertexCover(int n, const vector<pair<int,int>>& edges);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <graph_file>\n";
        return 1;
    }

    ifstream fin(argv[1]);
    if (!fin) {
        cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    // Remap arbitrary node IDs to 0..n-1
    unordered_map<int,int> id2idx;
    vector<pair<int,int>> edges;
    string line;

    auto getIdx = [&](int raw) -> int {
        auto [it, inserted] = id2idx.emplace(raw, (int)id2idx.size());
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
    int m = (int)edges.size();
    cout << "Nodes: " << n << "  Edges: " << m << "\n";

    auto t0 = chrono::steady_clock::now();
    vector<int> cover = approxVertexCover(n, edges);
    auto t1 = chrono::steady_clock::now();

    double ms = chrono::duration<double, milli>(t1 - t0).count();
    cout << "Cover size: " << cover.size() << "\n";
    cout << "Time: " << fixed << setprecision(3) << ms << " ms\n";

    return 0;
}
