#pragma once

#include <upcxx/upcxx.hpp>
#include <upcxx/atomic.hpp>
// #include "definitions.h"

struct DistributedUnionFind {
    //parents and (node) ranks for this (compute) rank
    upcxx::dist_object<upcxx::global_ptr<uint64_t>> parents;
    upcxx::dist_object<upcxx::global_ptr<uint64_t>> ranks;

    upcxx::global_ptr<int> num_sets;

    upcxx::atomic_domain<uint64_t> ad_uint;
    upcxx::atomic_domain<int> ad_int;

    // total number of nodes
    uint64_t N, normal_chunk_size;
    // the start and end indices of the nodes stored on this rank.
    int local_start, local_end;

    
    // helper functions
    int local_index(int x, int rank) const { return x - rank * normal_chunk_size; }

    bool is_local(int x) const {
        return x >= local_start && x < local_end;
    }

    DistributedUnionFind(int n);

    upcxx::global_ptr<uint64_t> to_ptr(uint64_t i);

    int owner(uint64_t i); //find the compute rank that i belongs to 
    uint64_t get_parent(uint64_t i); //find the direct parent's index for i (1 layer up the "tree")
    void set_parent(uint64_t i, uint64_t j);
    uint64_t get_rank(uint64_t i);
    void increment_rank(uint64_t i);
    
    bool merge(uint64_t i, uint64_t j);
    uint64_t find(uint64_t i);
    int get_num_sets();
    void print();
    void destroy();
};

DistributedUnionFind::DistributedUnionFind(int n) : ad_int({upcxx::atomic_op::add}), ad_uint({upcxx::atomic_op::add}) {
    int P = upcxx::rank_n();
    int rank = upcxx::rank_me();
    int chunk_size = (n + P - 1) / P;
    N = static_cast<uint64_t>(n);
    normal_chunk_size = static_cast<uint64_t>(chunk_size);

    // broadcast num_sets to all ranks
    if (upcxx::rank_me() == 0) {
        num_sets = upcxx::new_<int>(n);
    }
    num_sets = upcxx::broadcast(num_sets, 0).wait();

    // this rank will store [local_start, local_end)
    local_start = rank * chunk_size;
    local_end = std::min(n, (rank + 1) * chunk_size);

    int local_size = local_end - local_start;

    parents = upcxx::dist_object<upcxx::global_ptr<uint64_t>>(upcxx::new_array<uint64_t>(local_size));
    ranks = upcxx::dist_object<upcxx::global_ptr<uint64_t>>(upcxx::new_array<uint64_t>(local_size));

    uint64_t* local_parents = parents->local();
    //fill the local parents array 
    for (int i = 0; i < local_size; i++) {
        local_parents[i] = local_start + i;
    }
    
};

upcxx::global_ptr<uint64_t> DistributedUnionFind::to_ptr(uint64_t i) {
    int node_owner = owner(i);
    int local_idx = local_index(i, node_owner);
    return parents.fetch(node_owner).wait() + local_idx;
}

int DistributedUnionFind::owner(uint64_t i) {
    int num_ranks = upcxx::rank_n();
    return std::min(static_cast<int>(i / normal_chunk_size), num_ranks - 1);
};

uint64_t DistributedUnionFind::get_parent(uint64_t i) {
    // RPC find on actual rank
    upcxx::global_ptr<uint64_t> ptr = to_ptr(i);

    if (is_local(i)) {
        // local get
        return ptr.local()[0];
    } else {
        // remote get
        return upcxx::rget(ptr).wait(); 
    }
};

void DistributedUnionFind::set_parent(uint64_t i, uint64_t j) {
    //set the parent array for node i to j (in local terms, parent[i] = j)
    upcxx::global_ptr<uint64_t> ptr = to_ptr(i);

    if (is_local(i)) {
        // local set
        ptr.local()[0] = j;
    } else {
        // remote set - we can choose to remove wait() if we don't care about correctness
        upcxx::rput(j, ptr).wait();
    }
}

uint64_t DistributedUnionFind::get_rank(uint64_t i) {
    // RPC find on actual rank
    upcxx::global_ptr<uint64_t> ptr = to_ptr(i);

    if (is_local(i)) {
        // local get
        return ptr.local()[0];
    } else {
        // remote get
        return upcxx::rget(ptr).wait(); 
    }
};

void DistributedUnionFind::increment_rank(uint64_t i) {
    //set the rank for node i by 1 (in local terms, ranks[i]++)
    int node_owner = owner(i);
    upcxx::global_ptr<uint64_t> ptr = to_ptr(i);

    if (is_local(i)) {
        // local set
        ptr.local()[0]++;
    } else {
        // remote set - we can choose to remove wait() if we don't care about correctness
        ad_uint.add(ptr, 1, std::memory_order_relaxed).wait();
    }
}

uint64_t DistributedUnionFind::find(uint64_t i) {
    printf("Finding node %d from rank %d", i, upcxx::rank_me());
    std::vector<uint64_t> path;

    // traverse to the root of this subtree
    while (true) {
        uint64_t parent = get_parent(i);
        if (parent == i) break;
        path.push_back(i);
        i = parent;
    }

    // set all parents along path back to i
    for (uint64_t node : path) {
        set_parent(node, i);
    }

    //i should be root now
    return i;
}

bool DistributedUnionFind::merge(uint64_t i, uint64_t j) {
    uint64_t pi = find(i);
    uint64_t pj = find(j);
    if (pi == pj) return false;
    if (get_rank(pi) < get_rank(pj)) {
        set_parent(pi, pj);
    } else if (get_rank(pj) < get_rank(pi)) {
        set_parent(pj, pi);
    } else {
        set_parent(pj, pi);
        increment_rank(pi);
    }
    ad_int.add(num_sets, -1, std::memory_order_relaxed).wait();;
    return true;
}

int DistributedUnionFind::get_num_sets() {
    return upcxx::rget(num_sets).wait();
}

void DistributedUnionFind::print() {
    printf("Rank %d nodes:", upcxx::rank_me());
    for (int i = 0; i < local_end - local_start; i++) {
        printf("%d ", parents->local()[i]);
    }
    printf("\n");
}

void DistributedUnionFind::destroy() {
    if (upcxx::rank_me() == 0)
        upcxx::delete_(num_sets);

    ad_int.destroy();
    ad_uint.destroy(); // always clean up atomic_domain
}