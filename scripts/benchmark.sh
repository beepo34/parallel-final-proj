#!/bin/bash

# correct usage to run on one node, two hours in cpu:
# salloc -N 1 -A mp309 -t 120:00 --qos=interactive -C cpu bash scripts/benchmark.sh [run_name]


# make sure user provided a name for this run
if [ -z "$1" ]; then
  echo "Please provide a name for this run."
  exit -1
else
  run_name=$1
fi

num_cores=1
source modules.sh
mkdir -p scripts/out/upxx 
mkdir -p scripts/out/viecut

# run our implementation capforest
mkdir "/global/homes/a/azhangji/CS267/parallel-final-proj/scripts/out/$run_name"
export UPCXX_SHARED_HEAP_SIZE=4G
cd build
echo "#####UPCXX CAPFOREST#####"
for file_name in small small-dense med-sparse med med-dense large-sparse large; do
    for ntasks_per in 1 2 4 8 16 32 64 128; do
        echo "Running -N $num_cores --ntasks $ntasks_per on $file_name"
        echo "-N $num_cores --ntasks $ntasks_per --graph $file_name" >> "/global/homes/a/azhangji/CS267/parallel-final-proj/scripts/out/$run_name/upcxx-$file_name.out"
        srun -N $num_cores --ntasks-per-node $ntasks_per -C cpu ./mincut "$SCRATCH/random_hyperbolic_graphs/testing_graphs/strong/$file_name.metis" >> "/global/homes/a/azhangji/CS267/parallel-final-proj/scripts/out/$run_name/upcxx-$file_name.out"
    done
done

# run the viecut capforest
# cd /global/homes/a/azhangji/CS267/parallel-final-proj/VieCut/build
# echo "#####VieCut CAPFOREST#####"
# mkdir "/global/homes/a/azhangji/CS267/parallel-final-proj/scripts/out/$run_name"
# for file_name in small-sparse small small-dense med-sparse med med-dense large-sparse large; do
#     for nthreads in 1 2 4 8 16 32 64 128; do
#         echo "Running -n $num_cores --threads $nthreads --graph $file_name" | tee -a "/global/homes/a/azhangji/CS267/parallel-final-proj/scripts/out/$run_name/vie-$file_name.out"
#         export OMP_NUM_THREADS=$nthreads
#         srun -n $num_cores --cpus-per-task=$nthreads ./profiling_capforest_parallel "$SCRATCH/random_hyperbolic_graphs/testing_graphs/strong/$file_name.metis" $nthreads >> "/global/homes/a/azhangji/CS267/parallel-final-proj/scripts/out/$run_name/vie-$file_name.out"
#     done
# done
