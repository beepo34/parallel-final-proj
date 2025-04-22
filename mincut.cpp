#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <upcxx/upcxx.hpp>

#include "butil.hpp"

int main(int argc, char** argv) {
    upcxx::init();

    if (argc < 2) {
        BUtil::print("usage: srun -N nodes -n ranks ./mincut graph_file\n");
        upcxx::finalize();
        exit(1);
    }

    std::string graph_fname = std::string(argv[1]);

    // TODO: initialization

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