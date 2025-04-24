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

    // read in graph file name
    std::string graph_fname = std::string(argv[1]);


    upcxx::barrier(); // BARRIER (end of arg read)


    uint64_t num_nodes, num_edges, weight, size_per_rank;

    // read in the graph
    if (upcxx::rank_me() == 0) {
        std::string line;
        std::ifstream fin(graph_fname);

        getline(fin, line);

        std::stringstream ss(line);
        ss >> num_nodes;
        ss >> num_edges;
        ss >> weight;

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


    upcxx::barrier(); // BARRIER (end of graph init)

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
        uint64_t max_degree = 0;

        for (int i = 0; i < upcxx::rank_n(); i++) {
            for (int j = 0; j < size_per_rank; j++) {
                getline(fin, line);
                
                std::stringstream ss(line);
                uint64_t dst;
                uint64_t degree = 0;

                send_nodes.push_back({edge_counter, 0, false});

                while (ss >> dst) {
                    send_edges.push_back({dst - 1, weight});
                    edge_counter++;
                    degree++;
                }

                node_counter++;

                if (degree > max_degree) {
                    max_degree = degree;
                }

                if (fin.eof()) {
                    break;
                }
            }
            
            if (i > 0) {
                // insert into remote graph
                upcxx::future<> f = upcxx::rpc(
                    i,
                    [](dobj_nodes &lnodes, dobj_edges &ledges, std::vector<Node> nodes, std::vector<Edge> edges, dobj_visited &lvisited) {
                        (*lnodes) = upcxx::new_array<Node>(nodes.size());
                        (*ledges) = upcxx::new_array<Edge>(edges.size());
                        (*lvisited) = upcxx::new_array<bool>(nodes.size());
                        
                        std::copy(nodes.begin(), nodes.end(), lnodes->local());
                        std::copy(edges.begin(), edges.end(), ledges->local());
                    },
                    graph.nodes, graph.edges, send_nodes, send_edges, graph.visited
                );

                send_nodes.clear();
                send_edges.clear();

                f.wait();
            }
            else {
                // insert into local graph
                (*graph.nodes) = upcxx::new_array<Node>(send_nodes.size());
                (*graph.edges) = upcxx::new_array<Edge>(send_edges.size());
                (*graph.visited) = upcxx::new_array<bool>(send_nodes.size());

                std::copy(send_nodes.begin(), send_nodes.end(), graph.nodes->local());
                std::copy(send_edges.begin(), send_edges.end(), graph.edges->local());

                send_nodes.clear();
                send_edges.clear();
            }
        }

        // set lambda
        graph.lambda = upcxx::new_<uint64_t>(max_degree);
    }

    // broadcast lambda global_ptr
    graph.lambda = upcxx::broadcast(graph.lambda, 0).wait();

    auto end_io = std::chrono::high_resolution_clock::now();


    upcxx::barrier(); // BARRIER (end of graph read)
    double duration_io = std::chrono::duration<double>(end_io - start_io).count();
    BUtil::print("IO time: %f\n", duration_io);
    // for testing: each rank prints out lambda
    std::cout << "rank " << upcxx::rank_me() << " lambda: " << upcxx::rget(graph.lambda).wait() << std::endl;
    upcxx::barrier();


    auto start_work = std::chrono::high_resolution_clock::now();

    // TODO: work

    auto end_work = std::chrono::high_resolution_clock::now();


    upcxx::barrier(); // BARRIER (end of work)
    double duration_work = std::chrono::duration<double>(end_work - start_work).count();
    BUtil::print("Runtime: %f\n", duration_work);
    upcxx::barrier();


    upcxx::finalize();
    return 0;
}