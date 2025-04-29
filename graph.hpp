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
#include <queue>

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


struct GraphSection {
    // shared data
    upcxx::global_ptr<Node> nodes;
    upcxx::global_ptr<Edge> edges;
    upcxx::global_ptr<bool> visited;

    // global data
    upcxx::global_ptr<uint64_t> lambda;

    // local data
    std::unordered_set<uint64_t> blacklist;
    std::priority_queue<std::pair<uint64_t, uint64_t>> pq;
    uint64_t size;
    uint64_t local_num_nodes;
    uint64_t local_num_edges;

    GraphSection(uint64_t size) 
    :   size(size),
        nodes(nullptr), edges(nullptr), visited(nullptr) {};
    
    ~GraphSection() {
        if (nodes) upcxx::delete_array(nodes);
        if (edges) upcxx::delete_array(edges);
        if (visited) upcxx::delete_array(visited);
    
        if (upcxx::rank_me() == 0) {
            if (lambda) upcxx::delete_(lambda);
        }
    };

    // helpers
    uint64_t get_firstedge(uint64_t node) {
        if (node == local_num_nodes) {
            return local_num_edges;
        }
    
        Node* nodes_local = nodes.local();
        return nodes_local[node].firstedge;
    };

    int get_degree(uint64_t node) {
        UPCXX_ASSERT(node < local_num_nodes);
    
        uint64_t firstedge = get_firstedge(node);
        uint64_t lastedge = get_firstedge(node + 1);
        return lastedge - firstedge;
    };

    std::vector<Edge> get_edges(uint64_t node) {
        UPCXX_ASSERT(node < local_num_nodes);
    
        uint64_t firstedge = get_firstedge(node);
        uint64_t lastedge = get_firstedge(node + 1);
    
        Edge* edges_local = edges.local();
        return std::vector<Edge>(edges_local + firstedge, edges_local + lastedge);
    };
};


class Graph {

private:
    uint64_t num_nodes;
    uint64_t num_edges;
    uint64_t size_per_rank;

public:
    upcxx::dist_object<GraphSection> graphsection;

    Graph(uint64_t num_nodes, uint64_t num_edges, uint64_t size_per_rank) 
    : num_nodes(num_nodes), num_edges(num_edges), size_per_rank(size_per_rank),
      graphsection(GraphSection(size_per_rank)) {}
};