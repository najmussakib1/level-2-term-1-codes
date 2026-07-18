#include <iostream>
#include <vector>

class Solution {
public:
    vector<bool> checkIfPrerequisite(
        int n,
        vector<vector<int>>& prerequisites,
        vector<vector<int>>& queries) {

        vector<vector<bool>> reachable(n, vector<bool>(n, false));

        // Direct prerequisites
        for (auto &p : prerequisites)
            reachable[p[0]][p[1]] = true;

        // Warshall Algorithm
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    reachable[i][j] =
                        reachable[i][j] ||
                        (reachable[i][k] && reachable[k][j]);
                }
            }
        }

        vector<bool> ans;

        for (auto &q : queries)
            ans.push_back(reachable[q[0]][q[1]]);

        return ans;
    }
};