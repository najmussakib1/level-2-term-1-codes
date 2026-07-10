#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

const int INF = 1e9;

// Dijkstra Function
void dijkstra(int source,
              vector<vector<pair<int, int>>> &adj,
              vector<int> &dist,
              vector<int> &parent)
{
    int n = adj.size();

    dist.assign(n, INF);
    parent.assign(n, -1);

    priority_queue<pair<int, int>,
                   vector<pair<int, int>>,
                   greater<pair<int, int>>> pq;

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty())
    {
        int currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        // Ignore outdated entries
        if (currentDist > dist[u])
            continue;

        for (auto edge : adj[u])
        {
            int v = edge.first;
            int weight = edge.second;

            if (dist[u] + weight < dist[v])
            {
                dist[v] = dist[u] + weight;
                parent[v] = u;

                pq.push({dist[v], v});
            }
        }
    }
}

// Print shortest path from source to destination
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

    // adjacency list
    vector<vector<pair<int, int>>> adj(n);

    // Input: u v w
    for (int i = 0; i < m; i++)
    {
        int u, v, w;
        cin >> u >> v >> w;

        adj[u].push_back({v, w});
        adj[v].push_back({u, w});   // Remove this line for directed graph
    }

    int source;
    cin >> source;

    vector<int> dist;
    vector<int> parent;

    dijkstra(source, adj, dist, parent);

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