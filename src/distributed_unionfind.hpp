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

    upcxx::global_ptr<uint64_t> to_parent_ptr(uint64_t i);
    upcxx::global_ptr<uint64_t> to_rank_ptr(uint64_t i);

    int owner(uint64_t i); //find the compute rank that i belongs to 
    uint64_t get_parent(uint64_t i); //find the direct parent's index for i (1 layer up the "tree")
    void set_parent(uint64_t i, uint64_t j);
    uint64_t get_rank(uint64_t i);
    void increment_rank(uint64_t i);
    
    bool merge(uint64_t i, uint64_t j);
    uint64_t find(uint64_t i);
    bool update_root(uint64_t i, uint64_t ir, uint64_t y, uint64_t yr);

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

upcxx::global_ptr<uint64_t> DistributedUnionFind::to_parent_ptr(uint64_t i) {
    int node_owner = owner(i);
    int local_idx = local_index(i, node_owner);
    return parents.fetch(node_owner).wait() + local_idx;
}

upcxx::global_ptr<uint64_t> DistributedUnionFind::to_rank_ptr(uint64_t i) {
    int node_owner = owner(i);
    int local_idx = local_index(i, node_owner);
    return ranks.fetch(node_owner).wait() + local_idx;
}

int DistributedUnionFind::owner(uint64_t i) {
    int num_ranks = upcxx::rank_n();
    return std::min(static_cast<int>(i / normal_chunk_size), num_ranks - 1);
};

uint64_t DistributedUnionFind::get_parent(uint64_t i) {
    // RPC find on actual rank
    upcxx::global_ptr<uint64_t> ptr = to_parent_ptr(i);

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
    upcxx::global_ptr<uint64_t> ptr = to_parent_ptr(i);

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
    upcxx::global_ptr<uint64_t> ptr = to_rank_ptr(i);

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
    upcxx::global_ptr<uint64_t> ptr = to_rank_ptr(i);

    if (is_local(i)) {
        // local set
        ptr.local()[0]++;
    } else {
        // remote set - we can choose to remove wait() if we don't care about correctness
        ad_uint.add(ptr, 1, std::memory_order_relaxed).wait();
    }
}

uint64_t DistributedUnionFind::find(uint64_t i) {
    uint64_t parent = get_parent(i);

    // CAS path halving
    while (parent != i) {
        parent = get_parent(i);
        uint64_t grandparent = get_parent(parent);
        
        //does this work?
        ad_uint.compare_exchange(to_parent_ptr(i), parent, grandparent, std::memory_order_relaxed).wait();
        
        i = grandparent;
    }

    //i should be root now
    return i;
}

bool DistributedUnionFind::merge(uint64_t i, uint64_t j) {
    uint64_t i_rank, j_rank;

    while (true) {
        i = find(i);
        j = find(j);

        if (i == j) {
            return false;
        }

        i_rank = get_rank(i);
        j_rank = get_rank(j);

        if (i_rank > j_rank || (i_rank == j_rank && i > j)) {
            std::swap(i, j);
            std::swap(i_rank, j_rank);
        }

        if (update_root(i, i_rank, j, j_rank))
            break;
    
        // upcxx::progress();
    }

    if (i_rank == j_rank) { 
        //j rank increment
        update_root(j, j_rank, j, j_rank + 1);
    }

    ad_int.add(num_sets, -1, std::memory_order_relaxed).wait();
    return true;
}


// attempt to make the parent disjoint set i with rank i_rank to j with rank j_rank. 
bool DistributedUnionFind::update_root(uint64_t i, uint64_t i_rank, uint64_t j, uint64_t j_rank) {
    uint64_t old = i;
    uint64_t old_parent = get_parent(old);
    uint64_t old_rank = get_rank(old);

    if (old_parent != i || old_rank != i_rank) {
        return false;
    }

    if (ad_uint.compare_exchange(to_parent_ptr(i), old, j, std::memory_order_relaxed).wait() == old) {
        // printf("Compare %d parent to %d and replace with %d\n", i, old, j);
        // fflush(stdout);
        old_rank = get_rank(old);
        if (ad_uint.compare_exchange(to_rank_ptr(i), old_rank, j_rank, std::memory_order_relaxed).wait() == old_rank) {
            // printf("Updating %d, %d to %d, %d \n", i, i_rank, j, j_rank);
            // fflush(stdout);
            return true;
        }
        
        //if the op fails, atomically update back to old.
        ad_uint.compare_exchange(to_parent_ptr(i), j, old, std::memory_order_relaxed).wait();
    }
    return false;
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

    upcxx::delete_array(*parents);
    upcxx::delete_array(*ranks);

    ad_int.destroy();
    ad_uint.destroy(); // always clean up atomic_domain
}