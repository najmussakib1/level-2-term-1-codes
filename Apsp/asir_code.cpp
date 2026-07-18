//1334
class Solution {
public:
    vector<bool> checkIfPrerequisite(int n, vector<vector<int>>& p,
                                     vector<vector<int>>& q) {
        vector<vector<bool>> dist(n, vector<bool>(n, false));
        vector<bool> result;
        for (auto& c : p)
            dist[c[0]][c[1]] = true;
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) {
                for (int i = 0; i < n; i++) {
                    if (dist[i][k] && dist[k][j])
    dist[i][j] = true;
                }
            }
        }
        for (auto& x : q) {
            result.push_back(dist[x[0]][x[1]]);
        }
        return result;
    }
};

//2976
class Solution {
public:
    long long minimumCost(string src, string tgt, vector<char>& o,
                          vector<char>& c, vector<int>& cost) {
        int n = 26;
        vector<vector<long long>> dist(26, vector<long long>(26, LLONG_MAX));
        for (int i = 0; i < n; i++)
            dist[i][i] = 0;
        int f = static_cast<int>('a');
        for (int i = 0; i < o.size(); i++) {
           dist[o[i] - f][c[i] - f] =
    min(dist[o[i] - f][c[i] - f], (long long)cost[i]);
        }
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (dist[i][k] != LLONG_MAX && dist[k][j] != LLONG_MAX)
                        dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
                }
            }
        }
        long long result = 0;
        for (int i = 0; i < src.size(); i++) {
            if (dist[src[i] - f][tgt[i] - f] == LLONG_MAX)
                return -1;
            result += dist[(int)src[i] - f][(int)tgt[i] - f];
        }
        return result;
    }
};

//base_code

#include <bits/stdc++.h>
using namespace std;

const long long INF = 1e18;

int main() {
    int n, m;
    cin >> n >> m;

    vector<vector<long long>> dist(n, vector<long long>(n, INF));

    // Distance from a node to itself is 0
    for (int i = 0; i < n; i++)
        dist[i][i] = 0;

    // Input edges (0-based indexing)
    for (int i = 0; i < m; i++) {
        int u, v;
        long long w;
        cin >> u >> v >> w;

        dist[u][v] = min(dist[u][v], w);

        // Uncomment if the graph is undirected
        // dist[v][u] = min(dist[v][u], w);
    }

    // Floyd-Warshall
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; j++) {
                if (dist[k][j] == INF) continue;

                dist[i][j] = min(dist[i][j],
                                 dist[i][k] + dist[k][j]);
            }
        }
    }

    // Check for negative cycle
    for (int i = 0; i < n; i++) {
        if (dist[i][i] < 0) {
            cout << "Negative cycle exists\n";
            return 0;
        }
    }

    // Print shortest distance matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (dist[i][j] == INF)
                cout << "INF ";
            else
                cout << dist[i][j] << " ";
        }
        cout << '\n';
    }

    return 0;
}