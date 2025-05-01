#include <unordered_set>
#include <vector>
#include <queue>
#include <cstdlib>

#include "graph.hpp"
#include "unionfind.hpp"

void capforest(Graph& graph, upcxx::dist_object<UnionFind>& uf, upcxx::global_ptr<uint64_t>& lambda) {
    std::unordered_set<uint64_t> blacklist;
    std::priority_queue<std::pair<uint64_t, uint64_t>> pq; // TODO: custom pq
    std::vector<uint64_t> r(graph.size());
    std::vector<bool> local_visited(graph.size());

    // shared global visited
    upcxx::dist_object<upcxx::global_ptr<bool>> visited = upcxx::new_array<bool>((graph.graphsection)->size);

    int offset = rand() % (graph.graphsection)->size + 1;
    int current_node = offset + graph.section_size() * upcxx::rank_me();

    pq.push({0, current_node}); // TODO

    while (!pq.empty()) {
        current_node = pq.top().second; // TODO
        pq.pop();

        int rank = graph.get_target_rank(current_node);
        int offset = current_node - (rank * graph.section_size());

        upcxx::future<> f;
        if (rank == upcxx::rank_me()) {
            visited->local()[offset] = true;
        }
        else {
            upcxx::global_ptr<bool> visited_ptr = visited.fetch(rank).wait();
            bool value = true;
            upcxx::rput(value, visited_ptr + offset).wait();
        }

        local_visited[current_node] = true;
        blacklist.insert(current_node);

        std::vector<Edge> edges = graph.get_edges(current_node);
        for (auto& edge : edges) {
            uint64_t dst = edge.dst;
            uint64_t weight = edge.weight;
            if (!local_visited[dst]) {
                uint64_t mincut = upcxx::rget(lambda).wait(); // TODO: find better way
                if (r[dst] < mincut) {
                    if (r[dst] + weight >= mincut) {
                        if (!blacklist.count(dst)) {
                            upcxx::rpc(
                                0,
                                [](upcxx::dist_object<UnionFind>& uf, uint64_t src, uint64_t dst) {
                                    uf->merge(src, dst);
                                }, uf, current_node, dst
                            ).wait();
                        }
                    }

                    int dst_rank = graph.get_target_rank(dst);
                    int dst_offset = dst - (dst_rank * graph.section_size());
                    bool dst_visited = false;
                    if (rank == upcxx::rank_me()) {
                        dst_visited = visited->local()[offset];
                    }
                    else {
                        upcxx::global_ptr<bool> visited_ptr = visited.fetch(rank).wait();
                        dst_visited = upcxx::rget(visited_ptr + offset).wait();
                    }

                    if (!dst_visited) {
                        uint64_t new_r = std::min(r[dst] + weight, mincut);
                        r[dst] = new_r;
                        // TODO: pq logic
                    }
                }
            }
        }
    }
}