#include<iostream>
#include<vector>
using namespace std;
const long long INF = 1e18;
int main(){
    int n,m;
    cin>>n>>m;
    int t;
    cin>>t;
    vector<vector<long long>> dist(n + 1, vector<long long>(n + 1, INF));

    // nijer moddhe distance 0.. diagonal borabor 0
    for (int i = 1; i <= n; i++){
        dist[i][i] = 0;
    }
    for(int i=0;i<m;i++){
        int u,v,w;
        cin>>u>>v>>w;
        dist[u][v] = w;
        dist[v][u]= w;
    }
    for (int k = 1; k <= n; k++)
    {
        for (int i = 1; i <= n; i++)
        {
            if (dist[i][k] == INF) continue;

            for (int j = 1; j <= n; j++)
            {
                if (dist[k][j] == INF) continue;

                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }
    vector<int> city_count (n,0);
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(dist[i+1][j+1]<=t && i!=j){
                city_count[i]++;
            }
        }
    }
    int min_idx = 0;
    int min = city_count[0];
    for(int i=1;i<n;i++){
        if(city_count[i]<=min){
            min = city_count[i];
            min_idx = i;
        }
    }
    cout<<"Min idx is "<<min_idx<<" min city is "<< min_idx+1<<" with "<<min<<" cities"<<endl;
}