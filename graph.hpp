#pragma once

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>

#include <upcxx/upcxx.hpp>

struct Node {
    uint64_t firstedge;
    uint32_t partition;
    bool in_cut;
};

struct Edge {
    uint64_t dst;
    uint64_t weight;
};


struct Graph {
    // shared data
    upcxx::global_ptr<Node> nodes;
    upcxx::global_ptr<Edge> edges;
    upcxx::global_ptr<bool> visited;

    // global data
    upcxx::global_ptr<uint64_t> lambda;

    // local data
    std::unordered_set<uint64_t> blacklist;
    uint64_t num_nodes;
    uint64_t num_edges;
    uint64_t size_per_rank;

    uint64_t local_num_nodes;
    uint64_t local_num_edges;

    Graph(uint64_t num_nodes, uint64_t num_edges, uint64_t size_per_rank);
    ~Graph();

    void capforest();

    // helpers
};

Graph::Graph(uint64_t num_nodes, uint64_t num_edges, uint64_t size_per_rank) 
    :   num_nodes(num_nodes), num_edges(num_edges), size_per_rank(size_per_rank),
        nodes(nullptr), edges(nullptr), visited(nullptr) {};

Graph::~Graph() {
    if (nodes) upcxx::delete_array(nodes);
    if (edges) upcxx::delete_array(edges);
    if (visited) upcxx::delete_array(visited);

    if (upcxx::rank_me() == 0) {
        if (lambda) upcxx::delete_(lambda);
    }
}

void Graph::capforest() {
    return;
}