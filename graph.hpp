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

using dobj_nodes = upcxx::dist_object<upcxx::global_ptr<Node>>;
using dobj_edges = upcxx::dist_object<upcxx::global_ptr<Edge>>;
using dobj_visited = upcxx::dist_object<upcxx::global_ptr<bool>>;

struct Graph {
    // shared data
    dobj_nodes nodes;
    dobj_edges edges;
    dobj_visited visited;

    // global data
    upcxx::global_ptr<uint64_t> lambda;

    // local data
    std::unordered_set<uint64_t> blacklist;
    uint64_t num_nodes;
    uint64_t num_edges;
    uint64_t size_per_rank;

    Graph(uint64_t num_nodes, uint64_t num_edges, uint64_t size_per_rank);
    // ~Graph();

    // void capforest();
};

Graph::Graph(uint64_t num_nodes, uint64_t num_edges, uint64_t size_per_rank) 
    :   num_nodes(num_nodes), num_edges(num_edges), size_per_rank(size_per_rank) {};

// Graph::~Graph() {
//     upcxx::delete_array(*nodes);
//     upcxx::delete_array(*edges);
//     upcxx::delete_array(*visited);
// }

// void Graph::capforest() {

// }