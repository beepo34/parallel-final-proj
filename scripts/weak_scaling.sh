#!/bin/bash

# correct usage:
# salloc -N 4 -A mp309 -t 15:00 --qos=interactive -C cpu bash scripts/weak_scaling.sh [run_name]

if [ -z "$1" ]; then
  echo "Please provide a name for this run."
  exit -1
else
  run_name=$1
fi

num_cores=4
export UPCXX_SHARED_HEAP_SIZE=4G

source modules.sh
cd build

srun -N $num_cores --ntasks-per-node 1 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e16.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 2 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e17.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 4 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e18.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 8 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e19.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 16 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e20.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 32 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e21.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 64 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e22.metis" >> "$run_name.out"
srun -N $num_cores --ntasks-per-node 128 -C cpu ./mincut "$SCRATCH/testing_graphs/weak/2e23.metis" >> "$run_name.out"