#!/bin/bash

# correct usage:
# salloc -N 4 -A mp309 -t 60:00 --qos=interactive -C cpu bash scripts/intranode_scaling.sh [run_name]

if [ -z "$1" ]; then
  echo "Please provide a name for this run."
  exit -1
else
  run_name=$1
fi

num_cores=1
export UPCXX_SHARED_HEAP_SIZE=4G

source modules.sh
cd build
for file_name in small-sparse small small-dense med-sparse med med-dense; do
    for ntasks_per in 1 2 4 8 16 32 64 128; do
        echo "Running -N $num_cores --ntasks $ntasks_per on $file_name"
        echo "-N $num_cores --ntasks $ntasks_per" >> "$run_name-$file_name".out
        srun -N $num_cores --ntasks-per-node $ntasks_per -C cpu ./mincut "$SCRATCH/testing_graphs/strong/$file_name.metis" >> "$run_name-$file_name".out
    done
done