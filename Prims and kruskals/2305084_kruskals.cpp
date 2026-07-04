// #include <bits/stdc++.h>
#include <iostream>
#include <vector>

using namespace std;
class Disjoint_Set_Union
{
private:
    vector<long long> parent;
    vector<long long> rank;

public:
    Disjoint_Set_Union(long long n)
    {
        parent.resize(n);
        rank.resize(n, 0);
        for (long long i = 0; i < n; i++)
        {
            parent[i] = i;
        }
    }
    long long find(long long x)
    {
        if (parent[x] != x)
        {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    bool unite_node(long long x, long long y)
    {
        long long rootX = find(x);
        long long rootY = find(y);
        if (rootX == rootY)
        {
            return false;
        }

        if (rank[rootX] < rank[rootY])
        {
            parent[rootX] = rootY;
        }
        else if (rank[rootX] > rank[rootY])
        {
            parent[rootY] = rootX;
        }
        else
        {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
        return true;
    }
};
bool compare_edges(const pair<long long, pair<long long, long long>> &a, const pair<long long, pair<long long, long long>> &b)
{
    if (a.first != b.first)
        return a.first < b.first;
    if (a.second.first != b.second.first)
        return a.second.first < b.second.first;

    return a.second.second < b.second.second;
}
void kruskal_algorithm(vector<pair<long long, pair<long long, long long>>> &edges, long long n)
{
    sort(edges.begin(), edges.end(), compare_edges);

    Disjoint_Set_Union dsu(n);
    vector<pair<long long, long long>> MSTedges;
    long long totalWeight = 0;

    for (vector<pair<long long, pair<long long, long long>>>::iterator it = edges.begin(); it != edges.end(); it++)
    {
        long long weight = it->first;
        long long u = it->second.first;
        long long v = it->second.second;

        if (dsu.unite_node(u, v))
        {
            MSTedges.push_back(make_pair(u, v));
            totalWeight += weight;
        }
    }
    cout << "Total weight " << totalWeight << endl;
    for (vector<pair<long long, long long>>::iterator it = MSTedges.begin(); it != MSTedges.end(); ++it)
    {
        pair<long long, long long> mst_edge = *it;
        cout << mst_edge.first << " " << mst_edge.second << endl;
    }
}

int main()
{
    long long n, m;
    cin >> n >> m;

    vector<vector<pair<long long, long long>>> adj(n);
    vector<pair<long long, pair<long long, long long>>> edges;

    for (long long i = 0; i < m; i++)
    {
        long long u, v, w;
        cin >> u >> v >> w;

        adj[u].push_back(make_pair(v, w));
        adj[v].push_back(make_pair(u, w));

        edges.push_back(make_pair(w, make_pair(u, v)));
    }

    long long root;
    cin >> root;

    kruskal_algorithm(edges, n);
}