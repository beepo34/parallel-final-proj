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

using dobj_nodes = upcxx::dist_object<std::vector<Node>>;
using dobj_edges = upcxx::dist_object<std::vector<Edge>>;

struct Graph {
    dobj_nodes nodes;
    dobj_edges edges;

    uint64_t global_num_nodes;
    uint64_t global_num_edges;
    size_t start_node;
    size_t end_node;

    Graph() : nodes(std::vector<Node>()), edges(std::vector<Edge>()) {};
};
