#pragma once

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

    uint64_t size;
    uint64_t local_num_nodes;
    uint64_t local_num_edges;

    GraphSection(uint64_t size) 
    :   size(size),
        nodes(nullptr), edges(nullptr) {};
    
    ~GraphSection() {
        if (nodes) upcxx::delete_array(nodes);
        if (edges) upcxx::delete_array(edges);
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

    size_t size() const noexcept { return num_nodes; }
    size_t section_size() const noexcept { return size_per_rank; }

    std::vector<Edge> get_edges(uint64_t node) {
        int rank = get_target_rank(node);
        int offset = node - (rank * size_per_rank);
        if (rank == upcxx::rank_me()) {
            return graphsection->get_edges(offset);
        }
        else {
            return upcxx::rpc(
                rank,
                [](upcxx::dist_object<GraphSection>& graphsection, int offset) -> std::vector<Edge> {
                    return graphsection->get_edges(offset);
                }, graphsection, offset
            ).wait();
        }
    }

    int get_target_rank(uint64_t node) {
        return static_cast<int>(node / size_per_rank);
    }
};