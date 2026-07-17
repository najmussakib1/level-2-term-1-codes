#include<iostream>
#include<vector>

using namespace std;

const long long INF = 1e18;

int main()
{
    int n, m;
    cin >> n >> m;

    // Distance matrix
    vector<vector<long long>> dist(n + 1, vector<long long>(n + 1, INF));

    // nijer moddhe distance 0.. diagonal borabor 0
    for (int i = 1; i <= n; i++){
        dist[i][i] = 0;
    }


    // Read edges
    for (int i = 0; i < m; i++)
    {
        int u, v;
        long long w;
        cin >> u >> v >> w;

        // For directed graph
        dist[u][v] = min(dist[u][v], w);

        // Uncomment for undirected graph
        // dist[v][u] = min(dist[v][u], w);
    }

    // Floyd-Warshall Algorithm
    for (int k = 1; k <= n; k++)
    {
        for (int i = 1; i <= n; i++)
        {
            if (dist[i][k] == INF)
                continue;

            for (int j = 1; j <= n; j++)
            {
                if (dist[k][j] == INF)
                    continue;

                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }

    // Print shortest distances
    cout << "Shortest Distance Matrix:\n";

    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (dist[i][j] == INF)
                cout << "INF ";
            else
                cout << dist[i][j] << " ";
        }
        cout << '\n';
    }

    return 0;
}