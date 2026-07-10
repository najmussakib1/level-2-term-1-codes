#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

const int INF = 1e9;

// Bellman-Ford Function
bool bellmanFord(int source,
                 vector<vector<pair<int, int>>> &adj,
                 vector<int> &dist,
                 vector<int> &parent)
{
    int n = adj.size();

    dist.assign(n, INF);
    parent.assign(n, -1);

    dist[source] = 0;

    // Relax all edges (V-1) times
    for (int i = 1; i <= n - 1; i++)
    {
        bool updated = false;

        for (int u = 0; u < n; u++)
        {
            if (dist[u] == INF)
                continue;

            for (auto edge : adj[u])
            {
                int v = edge.first;
                int weight = edge.second;

                if (dist[u] + weight < dist[v])
                {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    updated = true;
                }
            }
        }

        // Optimization: Stop if no updates
        if (!updated)
            break;
    }

    // Check for negative weight cycle
    for (int u = 0; u < n; u++)
    {
        if (dist[u] == INF)
            continue;

        for (auto edge : adj[u])
        {
            int v = edge.first;
            int weight = edge.second;

            if (dist[u] + weight < dist[v])
            {
                return false; // Negative cycle exists
            }
        }
    }

    return true;
}

// Print shortest path
void printPath(int destination, vector<int> &parent)
{
    if (destination == -1)
        return;

    printPath(parent[destination], parent);
    cout << destination << " ";
}

int main()
{
    int n, m;
    cin >> n >> m;

    vector<vector<pair<int, int>>> adj(n);

    // Input: u v weight
    // Directed Graph
    for (int i = 0; i < m; i++)
    {
        int u, v, w;
        cin >> u >> v >> w;

        adj[u].push_back({v, w});
    }

    int source;
    cin >> source;

    vector<int> dist;
    vector<int> parent;

    bool ok = bellmanFord(source, adj, dist, parent);

    if (!ok)
    {
        cout << "Negative Weight Cycle Detected!\n";
        return 0;
    }

    cout << "\nShortest Distances:\n";

    for (int i = 0; i < n; i++)
    {
        cout << source << " -> " << i << " = ";

        if (dist[i] == INF)
            cout << "INF";
        else
            cout << dist[i];

        cout << endl;
    }

    cout << "\nShortest Paths:\n";

    for (int i = 0; i < n; i++)
    {
        cout << source << " -> " << i << " : ";

        if (dist[i] == INF)
        {
            cout << "No Path";
        }
        else
        {
            printPath(i, parent);
        }

        cout << endl;
    }

    return 0;
}