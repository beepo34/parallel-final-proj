#!/bin/bash

# Correct Usage:
# salloc -N 1 -A mp309 -t 30:00 --qos=interactive -C cpu bash scripts/intranode_scaling.sh [run_name]

if [ -z "$1" ]; then
  echo "Please provide a name for this run."
  exit -1
else
  run_name=$1
fi

num_cores=1
cd ./build
for proc_count in 1 2 4 8 16 32 64 128; do
    echo "$proc_count tasks:" >> intranode.out
    UPCXX_SHARED_HEAP_SIZE=4G srun -N "$num_cores" --ntasks-per-node "$proc_count" -C cpu ./mincut "$SCRATCH/data" >> "$run_name-small".out
done