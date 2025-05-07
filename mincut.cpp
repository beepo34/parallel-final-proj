#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <upcxx/upcxx.hpp>

#include "src/butil.hpp"
#include "src/graph.hpp"
#include "src/distributed_unionfind.hpp"
#include "src/mincut.hpp"

// #if 0
// salloc -N 1 -A mp309 -t 10:00 --qos=interactive -C cpu srun -N 1 --ntasks-per-node 4 ./mincut ../graphs/small.metis
// #endif 

int main(int argc, char** argv) {
    upcxx::init();

    if (argc < 2) {
        BUtil::print("usage: srun -N nodes -n ranks ./mincut graph_file\n");
        upcxx::finalize();
        exit(1);
    }

    // read in graph file name
    std::string graph_fname = std::string(argv[1]);


    upcxx::barrier(); // BARRIER (end of arg read)


    uint64_t num_nodes = 0, num_edges = 0, size_per_rank = 0;
    uint64_t has_weight = 0;

    // read in the graph
    if (upcxx::rank_me() == 0) {
        std::string line;
        std::ifstream fin(graph_fname);

        getline(fin, line);

        std::stringstream ss(line);
        ss >> num_nodes;
        ss >> num_edges;
        ss >> has_weight;

        size_per_rank = num_nodes / upcxx::rank_n();
        if (num_nodes % upcxx::rank_n() != 0) {
            size_per_rank++;
        }
    }

    // broadcast graph info
    num_nodes = upcxx::broadcast(num_nodes, 0).wait();
    num_edges = upcxx::broadcast(num_edges, 0).wait();
    size_per_rank = upcxx::broadcast(size_per_rank, 0).wait();

    // each rank will have a section of the graph
    Graph graph(num_nodes, num_edges, size_per_rank);
    
    // for now, union find on rank 0 only (size num_nodes) through rpc
    // if we have time, implement distributed union find where each rank has a local union find
    // and records cross-rank edges, merging all sets in a post-processing step
    upcxx::dist_object<DistributedUnionFind> unionfind{DistributedUnionFind(num_nodes)};

    uint64_t lambda = (uint64_t)-1;

    upcxx::barrier(); // BARRIER (end of init)


    auto start_io = std::chrono::high_resolution_clock::now();

    if (upcxx::rank_me() == 0) {
        std::string line;
        std::ifstream fin(graph_fname);

        // skip first line
        getline(fin, line);

        std::vector<Node> send_nodes;
        std::vector<Edge> send_edges;

        uint64_t node_counter = 0;
        uint64_t edge_counter = 0;

        // will be used to set lambda
        uint64_t min_degree = (uint64_t)-1;

        for (int i = 0; i < upcxx::rank_n(); i++) {
            // firstedge for each node will correspond to local index
            uint64_t local_edge_counter = 0;

            for (int j = 0; j < size_per_rank; j++) {
                getline(fin, line);
                
                std::stringstream ss(line);
                uint64_t dst;
                uint64_t weight;
                uint64_t degree = 0;

                send_nodes.push_back({local_edge_counter, 0, false});

                while (ss >> dst) {
                    if (has_weight == 0) { // hard code weight to 1
                        send_edges.push_back({dst - 1, 1});
                        degree++;
                    }
                    else {
                        ss >> weight;
                        send_edges.push_back({dst - 1, weight});
                        degree += weight;
                    }
                    local_edge_counter++;
                    edge_counter++;
                }

                node_counter++;

                if (degree < min_degree) {
                    min_degree = degree;
                }

                if (fin.eof()) {
                    break;
                }
            }

            // correctness checks
            UPCXX_ASSERT(send_edges.size() == local_edge_counter);
            if (i < upcxx::rank_n() - 1) {
                UPCXX_ASSERT(send_nodes.size() == size_per_rank);
            }
            
            if (i > 0) {
                // insert into remote graph
                upcxx::future<> f = upcxx::rpc(
                    i,
                    [](upcxx::dist_object<GraphSection> &lsection, std::vector<Node> nodes, std::vector<Edge> edges) {
                        lsection->nodes = upcxx::new_array<Node>(nodes.size());
                        lsection->edges = upcxx::new_array<Edge>(edges.size());
                        
                        std::copy(nodes.begin(), nodes.end(), (lsection->nodes).local());
                        std::copy(edges.begin(), edges.end(), (lsection->edges).local());

                        lsection->local_num_nodes = nodes.size();
                        lsection->local_num_edges = edges.size();
                    },
                    graph.graphsection, send_nodes, send_edges
                );

                send_nodes.clear();
                send_edges.clear();

                f.wait();
            }
            else {
                // insert into local graph
                upcxx::dist_object<GraphSection>& section = graph.graphsection;

                section->nodes = upcxx::new_array<Node>(send_nodes.size());
                section->edges = upcxx::new_array<Edge>(send_edges.size());
                
                std::copy(send_nodes.begin(), send_nodes.end(), (section->nodes).local());
                std::copy(send_edges.begin(), send_edges.end(), (section->edges).local());

                section->local_num_nodes = send_nodes.size();
                section->local_num_edges = send_edges.size();

                send_nodes.clear();
                send_edges.clear();
            }
        }

        // set lambda
        lambda = min_degree;
    }

    // broadcast lambda global_ptr
    lambda = upcxx::broadcast(lambda, 0).wait();

    // correctness checks
    UPCXX_ASSERT(lambda != (uint64_t)-1);
    UPCXX_ASSERT(graph.size() == num_nodes);
    UPCXX_ASSERT(graph.section_size() == size_per_rank);

    auto end_io = std::chrono::high_resolution_clock::now();


    upcxx::barrier(); // BARRIER (end of graph read)
    double duration_io = std::chrono::duration<double>(end_io - start_io).count();
    BUtil::print("IO time: %f\n", duration_io);
    BUtil::print("Number of nodes: %lu, Number of edges: %lu\n", num_nodes, num_edges);
    BUtil::print("Running CAPFOREST on graph with minimum cut %lu\n", lambda);
    upcxx::barrier();


    auto start_work = std::chrono::high_resolution_clock::now();

    capforest(graph, unionfind, lambda);

    auto end_work = std::chrono::high_resolution_clock::now();


    upcxx::barrier(); // BARRIER (end of work)
    double duration_work = std::chrono::duration<double>(end_work - start_work).count();
    BUtil::print("Runtime: %f\n", duration_work);
    BUtil::print("Marked contractions: %d\n", num_nodes - unionfind->get_num_sets());
    upcxx::barrier();


    upcxx::finalize();
    return 0;
}