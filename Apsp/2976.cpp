#include <iostream>
#include <vector>
#include <string>
using namespace std;

const long long INF = 1e18;

int main()
{
    string source, target;
    cin >> source >> target;

    int m;
    cin >> m;

    vector<vector<long long>> dist(26, vector<long long>(26, INF));

    // Distance from a character to itself is 0
    for (int i = 0; i < 26; i++)
        dist[i][i] = 0;

    // Read conversion rules
    for (int i = 0; i < m; i++)
    {
        char u, v;
        long long w;
        cin >> u >> v >> w;

        int from = u - 'a';
        int to = v - 'a';

        // Keep minimum cost if multiple rules exist
        dist[from][to] = min(dist[from][to], w);
    }

    // Floyd-Warshall
    for (int k = 0; k < 26; k++)
    {
        for (int i = 0; i < 26; i++)
        {
            if (dist[i][k] == INF)
                continue;

            for (int j = 0; j < 26; j++)
            {
                if (dist[k][j] == INF)
                    continue;

                dist[i][j] = min(dist[i][j],
                                 dist[i][k] + dist[k][j]);
            }
        }
    }

    long long totalCost = 0;

    for (int i = 0; i < source.size(); i++)
    {
        int from = source[i] - 'a';
        int to = target[i] - 'a';

        // Impossible conversion
        if (dist[from][to] == INF)
        {
            cout << -1 << endl;
            return 0;
        }

        totalCost += dist[from][to];
    }

    cout << totalCost << endl;

    return 0;
}