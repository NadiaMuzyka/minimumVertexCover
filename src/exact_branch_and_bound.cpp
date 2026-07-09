#include <bits/stdc++.h>
using namespace std;

// Brute-force exact Minimum Vertex Cover.
// Enumerates all 2^n subsets and returns the smallest that covers every edge.
// Feasible only for small n (n <= ~25 in practice).
vector<int> exactVertexCover(int n, const vector<pair<int,int>>& edges) {
    vector<int> best;

    for (int mask = 0; mask < (1 << n); ++mask) {
        // Prune: skip if this subset is already larger than best found
        if (!best.empty() && __builtin_popcount(mask) >= (int)best.size())
            continue;

        bool valid = true;
        for (auto [u, v] : edges) {
            if (!((mask >> u) & 1) && !((mask >> v) & 1)) {
                valid = false;
                break;
            }
        }

        if (valid) {
            best.clear();
            for (int i = 0; i < n; ++i)
                if ((mask >> i) & 1) best.push_back(i);
        }
    }

    return best;
}
