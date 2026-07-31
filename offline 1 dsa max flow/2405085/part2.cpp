#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
using namespace std;

struct Edge {
    int from, to, cap;
};

int n;
vector<Edge> edges;
vector<vector<int>> adj;

int addEdge(int u, int v, int w) {
    int edgeId = edges.size();
    adj[u].push_back(edgeId);
    edges.push_back({u, v, w});
    adj[v].push_back(edgeId + 1);
    edges.push_back({v, u, 0});
    return edgeId;
}

long long maxFlowEdmondsKarp(int s, int t) {
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
    return maxFlow;
}

int main() {
    int F;
    cin >> F;

    vector<string> id(F), depCity(F), arrCity(F);
    vector<int> depTime(F), arrTime(F);

    auto toMinutes = [](string t) {
        int hh = stoi(t.substr(0, 2));
        int mm = stoi(t.substr(3, 2));
        return hh * 60 + mm;
    };

    for (int i = 0; i < F; i++) {
        string dt, at;
        cin >> id[i] >> depCity[i] >> arrCity[i] >> dt >> at;
        depTime[i] = toMinutes(dt);
        arrTime[i] = toMinutes(at);
    }

    int source = 0;
    int sink = 2 * F + 1;
    n = 2 * F + 2;
    adj.assign(n, {});

    auto outNode = [&](int i) { return i + 1; };
    auto inNode  = [&](int i) { return F + i + 1; };

    const int TURNAROUND = 180;

    for (int i = 0; i < F; i++) {
        addEdge(source, outNode(i), 1);
        addEdge(inNode(i), sink, 1);
    }

    map<pair<int,int>, int> connectionEdge;

    for (int i = 0; i < F; i++) {
        for (int j = 0; j < F; j++) {
            if (i == j) continue;
            if (arrCity[i] == depCity[j] && arrTime[i] + TURNAROUND <= depTime[j]) {
                int edgeId = addEdge(outNode(i), inNode(j), 1);
                connectionEdge[{i, j}] = edgeId;
            }
        }
    }

    long long matching = maxFlowEdmondsKarp(source, sink);
    int numAircraft = F - (int)matching;


    vector<int> nextFlight(F, -1);
    vector<bool> hasIncoming(F, false);

    for (auto &[pair_ij, edgeId] : connectionEdge) {
        int i = pair_ij.first;
        int j = pair_ij.second;
        if (edges[edgeId].cap == 0) {
            nextFlight[i] = j;
            hasIncoming[j] = true;
        }
    }

    cout << "Number of Aircraft: " << numAircraft << "\n";
    int aircraftCount = 0;
    for (int i = 0; i < F; i++) {
        if (!hasIncoming[i]) {
            aircraftCount++;
            cout << "Aircraft " << aircraftCount << ": " << id[i];
            int cur = i;
            while (nextFlight[cur] != -1) {
                cur = nextFlight[cur];
                cout << " -> " << id[cur];
            }
            cout << "\n";
        }
    }

    return 0;
}