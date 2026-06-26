#include <iostream>
#include<vector>
#include<queue>
using namespace std;
int bfs(vector<vector<int>> &adj,vector<int>&dist,int initial, int final){
    queue<int> stor;
    stor.push(initial);
    dist[initial]=0;
    while(stor.size()!=0){
        int top=stor.front();
        for(auto x:adj[top]){
            if(dist[x]==-1){
                dist[x]==dist[top]+1;
                if(x==final){
                    return dist[x];
                }
            }
        }
    }
    return -1;
}
int main(){
    vector<vector<int>> adj;
    vector<int> dist(-1);
    int init=0;
    int final_=1;
    bfs(adj,dist,init,final_);
}