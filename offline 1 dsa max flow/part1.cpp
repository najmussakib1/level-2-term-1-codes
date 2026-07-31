#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;
struct Edge {
    int from, to, cap;
};

int main() {
    int n, m;
    cin >> n >> m;

    vector<Edge> edges;
    vector<vector<int>> adj(n);

    for (int i = 0; i < m; i++) {
        int u, v, w;
        cin >> u >> v >> w;
        adj[u].push_back(edges.size());
        edges.push_back({u, v, w});

        adj[v].push_back(edges.size());
        edges.push_back({v, u, 0});
    }

    int s = 0, t = n - 1;
    long long maxFlow = 0;

    while (true) {
        vector<int> parentEdge(n, -1);
        vector<bool> visited(n, false);
        queue<int> q;
        q.push(s);
        visited[s] = true;

        while (!q.empty() && !visited[t]) {
            int u = q.front(); q.pop();
            for (int edgeId : adj[u]) {
                int v = edges[edgeId].to;
                if (!visited[v] && edges[edgeId].cap > 0) {
                    visited[v] = true;
                    parentEdge[v] = edgeId;
                    q.push(v);
                }
            }
        }

        if (!visited[t]) break;

        int pathFlow = INT_MAX;
        for (int v = t; v != s; ) {
            int edgeId = parentEdge[v];
            pathFlow = min(pathFlow, edges[edgeId].cap);
            v = edges[edgeId].from;
        }
        for (int v = t; v != s; ) {
            int edgeId = parentEdge[v];
            edges[edgeId].cap -= pathFlow;
            edges[edgeId ^ 1].cap += pathFlow;
            v = edges[edgeId].from;
        }

        maxFlow += pathFlow;
    }

    cout << maxFlow << endl;
    return 0;
}