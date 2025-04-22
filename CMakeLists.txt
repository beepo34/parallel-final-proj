cmake_minimum_required(VERSION 3.14)
project(finalproj LANGUAGES CXX)

find_package(UPCXX REQUIRED)

# Build the mincut executable
add_executable(mincut mincut.cpp)
target_link_libraries(mincut PRIVATE UPCXX::upcxx)
# target_compile_definitions(kmer_hash_19 PRIVATE "KMER_LEN=19")