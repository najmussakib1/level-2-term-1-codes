#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
using namespace std;

struct edge {
    int from, to, cap;
};

int n;
vector<edge> edges;
vector<vector<int>> adj;

int addedge(int u, int v, int w) {
    int edgeid = edges.size();
    adj[u].push_back(edgeid);
    edges.push_back({u, v, w});
    adj[v].push_back(edgeid + 1);
    edges.push_back({v, u, 0});
    return edgeid;
}

long long maxflow(int s, int t) {
    long long maxflow = 0;
    while (true) {
        vector<int> parentedge(n, -1);
        vector<bool> visited(n, false);
        queue<int> q;
        q.push(s);
        visited[s] = true;

        while (!q.empty() && !visited[t]) {
            int u = q.front(); q.pop();
            for (int edgeid : adj[u]) {
                int v = edges[edgeid].to;
                if (!visited[v] && edges[edgeid].cap > 0) {
                    visited[v] = true;
                    parentedge[v] = edgeid;
                    q.push(v);
                }
            }
        }

        if (!visited[t]) break;

        int pathflow = INT_MAX;
        for (int v = t; v != s; ) {
            int edgeid = parentedge[v];
            pathflow = min(pathflow, edges[edgeid].cap);
            v = edges[edgeid].from;
        }

        for (int v = t; v != s; ) {
            int edgeid = parentedge[v];
            edges[edgeid].cap -= pathflow;
            edges[edgeid ^ 1].cap += pathflow;
            v = edges[edgeid].from;
        }

        maxflow += pathflow;
    }
    return maxflow;
}

int main() {
    int F;
    cin >> F;

    vector<string> id(F), departurecity(F), arrivalcity(F);
    vector<int> deptime(F), arrtime(F);

    auto convertminute = [](string t) {
        int hh = stoi(t.substr(0, 2));
        int mm = stoi(t.substr(3, 2));
        return hh * 60 + mm;
    };

    for (int i = 0; i < F; i++) {
        string dt, at;
        cin >> id[i] >> departurecity[i] >> arrivalcity[i] >> dt >> at;
        deptime[i] = convertminute(dt);
        arrtime[i] = convertminute(at);
    }

    int source = 0;
    int sink = 2 * F + 1;
    n = 2 * F + 2;
    adj.assign(n, {});

    auto outnode = [&](int i) { return i + 1; };
    auto innode  = [&](int i) { return F + i + 1; };

    const int TURNAROUND = 180;

    for (int i = 0; i < F; i++) {
        addedge(source, outnode(i), 1);
        addedge(innode(i), sink, 1);
    }

    map<pair<int,int>, int> connectionEdge;

    for (int i = 0; i < F; i++) {
        for (int j = 0; j < F; j++) {
            if (i == j) continue;
            if (arrivalcity[i] == departurecity[j] && arrtime[i] + TURNAROUND <= deptime[j]) {
                int edgeid = addedge(outnode(i), innode(j), 1);
                connectionEdge[{i, j}] = edgeid;
            }
        }
    }

    long long matching = maxflow(source, sink);
    int aircraftnumber = F - (int)matching;


    vector<int> nextflight(F, -1);
    vector<bool> incoming(F, false);

    for (auto &[pair_ij, edgeid] : connectionEdge) {
        int i = pair_ij.first;
        int j = pair_ij.second;
        if (edges[edgeid].cap == 0) {
            nextflight[i] = j;
            incoming[j] = true;
        }
    }

    cout << "Number of Aircraft: " << aircraftnumber << "\n";
    int aircraftcount = 0;
    for (int i = 0; i < F; i++) {
        if (!incoming[i]) {
            aircraftcount++;
            cout << "Aircraft " << aircraftcount << ": " << id[i];
            int cur = i;
            while (nextflight[cur] != -1) {
                cur = nextflight[cur];
                cout << " -> " << id[cur];
            }
            cout << "\n";
        }
    }

    return 0;
}