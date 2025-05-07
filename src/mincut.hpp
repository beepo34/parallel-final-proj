#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <random>

#include "graph.hpp"
// #include "unionfind.hpp"
#include "distributed_unionfind.hpp"
#include "priority_queue.hpp"

void capforest(Graph& graph, upcxx::dist_object<DistributedUnionFind>& uf, const uint64_t mincut, uint64_t seed = 0) {
    // std::unordered_set<uint64_t> blacklist;
    std::vector<uint64_t> r(graph.size());
    std::vector<int> local_visited(graph.size());
    fifo_node_bucket_pq pq(graph.size(), mincut);

    // shared global visited
    upcxx::dist_object<upcxx::global_ptr<int>> visited = upcxx::new_array<int>(graph.local_size());

    // each rank selects starting node
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(0, graph.local_size() - 1);
    int offset = dist(gen);
    int rank = upcxx::rank_me();
    int current_node = offset + graph.section_size() * rank;

    pq.insert(current_node, 0);

    while (!pq.empty()) {
        current_node = pq.deleteMax();
        
        local_visited[current_node] = 1;
        // blacklist.insert(current_node);

        int src_rank = graph.get_target_rank(current_node);
        int src_offset = current_node - (src_rank * graph.section_size());
        int src_visited = 0;
        if (src_rank == upcxx::rank_me()) {
            src_visited = visited->local()[src_offset];
            if (src_visited) {
                continue;
            }
            visited->local()[src_offset] = 1;
        }
        else {
            upcxx::global_ptr<int> visited_ptr = visited.fetch(src_rank).wait();
            src_visited = upcxx::rget(visited_ptr + src_offset).wait();
            if (src_visited) {
                continue;
            }
            upcxx::rput(1, visited_ptr + src_offset).wait();
        }

        std::vector<Edge> edges = graph.get_edges(current_node);
        for (auto& edge : edges) {
            uint64_t dst = edge.dst;
            uint64_t weight = edge.weight;
            
            if (!local_visited[dst]) {
                if (r[dst] < mincut) {
                    if (r[dst] + weight >= mincut) {
                        // if (!blacklist.count(dst)) {
                            // send to root rank for union
                        upcxx::rpc(
                            0,
                            [](upcxx::dist_object<DistributedUnionFind>& uf, uint64_t src, uint64_t dst) {
                                uf->merge(src, dst);
                            }, uf, current_node, dst
                        ).wait();
                        // }
                    }

                    int dst_rank = graph.get_target_rank(dst);
                    int dst_offset = dst - (dst_rank * graph.section_size());
                    int dst_visited = 0;
                    if (rank == upcxx::rank_me()) {
                        dst_visited = visited->local()[dst_offset];
                    }
                    else {
                        upcxx::global_ptr<int> visited_ptr = visited.fetch(dst_rank).wait();
                        dst_visited = upcxx::rget(visited_ptr + dst_offset).wait();
                    }

                    if (!dst_visited) {
                        uint64_t new_r = std::min(r[dst] + weight, mincut);
                        r[dst] = new_r;
                        if (pq.contains(dst)) {
                            pq.increaseKey(dst, new_r);
                        }
                        else {
                            pq.insert(dst, new_r);
                        }
                    }
                }
            }
        }
    }

    upcxx::barrier();

    // clean up
    upcxx::delete_array(*visited);
}