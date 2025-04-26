#pragma once

#include <vector>

struct UnionFind {
    std::vector<uint64_t> parent;
    std::vector<uint64_t> rank;

    int num_sets;

    UnionFind(int n);

    bool merge(uint64_t i, uint64_t j);
    uint64_t find(uint64_t i);
    int get_num_sets();
};

UnionFind::UnionFind(int n) : parent(n), rank(n), num_sets(n) {
    for (int i = 0; i < n; i++) {
        parent[i] = i;
        rank[i] = 0;
    }
};

bool UnionFind::merge(uint64_t i, uint64_t j) {
    uint64_t pi = find(i);
    uint64_t pj = find(j);
    if (pi == pj) return false;
    if (rank[pi] < rank[pj]) {
        parent[pi] = pj;
    } else if (rank[pi] > rank[pj]) {
        parent[pj] = pi;
    } else {
        parent[pj] = pi;
        rank[pi]++;
    }
    num_sets--;
    return true;
}

uint64_t UnionFind::find(uint64_t i) {
    while (parent[i] != i) {
        parent[i] = parent[parent[i]];
        i = parent[i];
    }
    return i;
}

int UnionFind::get_num_sets() {
    return num_sets;
}