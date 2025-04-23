#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <upcxx/upcxx.hpp>

#include "butil.hpp"
#include "graph.hpp"

int main(int argc, char** argv) {
    upcxx::init();

    if (argc < 2) {
        BUtil::print("usage: srun -N nodes -n ranks ./mincut graph_file\n");
        upcxx::finalize();
        exit(1);
    }

    std::string graph_fname = std::string(argv[1]);

    // Each rank will have a section of the graph
    Graph graph;

    upcxx::barrier();

    if (upcxx::rank_me() == 0) {
        uint64_t num_nodes, num_edges, weight;

        std::string line;
        std::ifstream fin(graph_fname);

        getline(fin, line);

        std::stringstream ss(line);
        ss >> num_nodes;
        ss >> num_edges;
        ss >> weight;

        uint64_t size_per_rank = num_nodes / upcxx::rank_n();
        if (num_nodes % upcxx::rank_n() != 0) {
            size_per_rank++;
        }

        std::vector<Node> send_nodes;
        std::vector<Edge> send_edges;

        uint64_t node_counter = 0;
        uint64_t edge_counter = 0;

        for (int i = 0; i < upcxx::rank_n(); i++) {
            for (int j = 0; j < size_per_rank; j++) {
                getline(fin, line);
                
                std::stringstream ss(line);
                uint64_t dst;

                send_nodes.push_back({edge_counter, 0, false});

                while (ss >> dst) {
                    send_edges.push_back({dst - 1, weight});
                    edge_counter++;
                }

                node_counter++;

                if (fin.eof()) {
                    break;
                }
            }
            
            if (i > 0) {
                upcxx::future<> f = upcxx::rpc(
                    i,
                    [](dobj_nodes &lnodes, dobj_edges &ledges, std::vector<Node> nodes, std::vector<Edge> edges) {
                        (*lnodes).insert((*lnodes).end(), nodes.begin(), nodes.end());
                        (*ledges).insert((*ledges).end(), edges.begin(), edges.end());
                    },
                    graph.nodes, graph.edges, send_nodes, send_edges
                );

                send_nodes.clear();
                send_edges.clear();

                f.wait();
            }
            else {
                (*graph.nodes).insert((*graph.nodes).end(), send_nodes.begin(), send_nodes.end());
                (*graph.edges).insert((*graph.edges).end(), send_edges.begin(), send_edges.end());

                send_nodes.clear();
                send_edges.clear();
            }
        }
    }
    
    upcxx::barrier();

    auto start = std::chrono::high_resolution_clock::now();

    // TODO: work

    auto end = std::chrono::high_resolution_clock::now();
    upcxx::barrier();

    // print duration
    double duration = std::chrono::duration<double>(end - start).count();
    BUtil::print("Time: %f\n", duration);
    upcxx::barrier();

    upcxx::finalize();
    return 0;
}