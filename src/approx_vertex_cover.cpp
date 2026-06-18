#include <bits/stdc++.h>
using namespace std;

// 2-approximation for Minimum Vertex Cover via maximal matching.
vector<int> approxVertexCover(int n, const vector<pair<int,int>>& edges) {
    vector<bool> covered(n, false);
    vector<int> cover;

    for (auto [u, v] : edges) {
        if (!covered[u] && !covered[v]) {
            covered[u] = true;
            covered[v] = true;
            cover.push_back(u);
            cover.push_back(v);
        }
    }

    return cover;
}


