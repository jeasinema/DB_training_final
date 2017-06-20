/*-----------------------------------------------------
 File Name :
 Purpose :
 Creation Date : 19-06-2017
 Last Modified : Tue 20 Jun 2017 02:33:58 AM CST
 Created By : Jeasine Ma [jeasinema[at]gmail[dot]com]
-----------------------------------------------------*/
#ifndef KM_HPP
#define KM_HPP 

#include <tuple>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>
#include <iostream>
#include <queue>

namespace km {

using namespace std;

int N = 305;
int INF = 2e9;

int n;
int **cost, *lx, *ly, *slack;
int *match, *prev;
bool *vy;

void init(int amount) {
    N = amount*2;
    n = amount;
    cost = new int*[N];
    for (int i = 0; i < N; ++i) {
        cost[i] = new int[N];
        memset(cost[i], 0, N*sizeof(int));
    }
    lx = new int[N];
    memset(lx, 0, N*sizeof(int));
    ly = new int[N];
    memset(ly, 0, N*sizeof(int));
    match = new int[N];
    memset(match, 0, N*sizeof(int));
    slack = new int[N];
    memset(slack, 0, N*sizeof(int));
    prev = new int[N];
    memset(prev, 0, N*sizeof(int));
    vy = new bool[N];
    memset(vy, 0, N*sizeof(bool));
}

void augment(int root) {
	std::fill(vy + 1, vy + n + 1, false);
	std::fill(slack + 1, slack + n + 1, INF);
	int py;
	match[py = 0] = root;
	do {
		vy[py] = true;
		int x = match[py], delta = INF, yy;
		for (int y = 1; y <= n; y++) {
			if (!vy[y]) {
				if (lx[x] + ly[y] - cost[x][y] < slack[y]) {
					slack[y] = lx[x] + ly[y] - cost[x][y];
					prev[y] = py;
				}
				if (slack[y] < delta) {
					delta = slack[y];
					yy = y;
				}
			}
		}
		for (int y = 0; y <= n; y++) {
			if (vy[y]) {
				lx[match[y]] -= delta;
				ly[y] += delta;
			} else {
				slack[y] -= delta;
			}
		}
		py = yy;
	} while (match[py] != -1);
	do {
		int pre = prev[py];
		match[py] = match[pre];
		py = pre;
	} while (py);
}

int KM() {
	for (int i = 1; i <= n; i++) {
		lx[i] = ly[i] = 0;
		match[i] = -1;
		for (int j = 1; j <= n; j++) {
			lx[i] = std::max(lx[i], cost[i][j]);
		}
	}
	int answer = 0;
	for (int root = 1; root <= n; root++) {
		augment(root);
	}
	for (int i = 1; i <= n; i++) {
		answer += lx[i];
		answer += ly[i];
		//printf("%d %d\n", match[i], i);
	}
	return answer;
}

int** get_global_edge() { 
    return cost; 
}

// passenger, driver, share_rate
vector<tuple<int, int, int>> get_perfect_match() {
    vector<tuple<int, int, int>> ret;
    int ans = KM();
    std::cout << "ans is " << ans << std::endl;
    
    for (int i = 1; i <= n; ++i) {
        ret.push_back(std::make_tuple(match[i], i, cost[match[i]][i]));
    }

    return ret;
}

}

#endif /* KM_HPP */
