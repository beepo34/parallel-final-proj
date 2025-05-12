#!/bin/bash

# Correct Usage:
# salloc -N 1 -A mp309 -C cpu -t 30:00 bash scripts/generate_test_graphs.sh 

module load conda
conda activate capforest-env

#strong scaling + intranode files
python3 util/generate_rhg.py -n 2 -e 15 -k 32 --file_path "$SCRATCH/testing_graphs/strong" --file_name "small-sparse"
python3 util/generate_rhg.py -n 2 -e 15 -k 32 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "small-sparse-w"
python3 util/generate_rhg.py -n 2 -e 15 -k 64 --file_path "$SCRATCH/testing_graphs/strong" --file_name "small"
python3 util/generate_rhg.py -n 2 -e 15 -k 64 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "small-w"
python3 util/generate_rhg.py -n 2 -e 15 -k 256 --file_path "$SCRATCH/testing_graphs/strong" --file_name "small-dense"
python3 util/generate_rhg.py -n 2 -e 15 -k 256 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "small-dense-w"
python3 util/generate_rhg.py -n 2 -e 20 -k 32 --file_path "$SCRATCH/testing_graphs/strong" --file_name "med-sparse"
python3 util/generate_rhg.py -n 2 -e 20 -k 32 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "med-sparse-w"
python3 util/generate_rhg.py -n 2 -e 20 -k 64 --file_path "$SCRATCH/testing_graphs/strong" --file_name "med"
python3 util/generate_rhg.py -n 2 -e 20 -k 64 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "med-w"
python3 util/generate_rhg.py -n 2 -e 20 -k 256 --file_path "$SCRATCH/testing_graphs/strong" --file_name "med-dense"
python3 util/generate_rhg.py -n 2 -e 20 -k 256 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "med-dense-w"
python3 util/generate_rhg.py -n 2 -e 25 -k 32 --file_path "$SCRATCH/testing_graphs/strong" --file_name "large-sparse"

#in practice these 5 seem to get killed when we try to run them (using too much memory?)
# python3 util/generate_rhg.py -n 2 -e 25 -k 32 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "large-sparse-w"
# python3 util/generate_rhg.py -n 2 -e 25 -k 64 --file_path "$SCRATCH/testing_graphs/strong" --file_name "large"
# python3 util/generate_rhg.py -n 2 -e 25 -k 64 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "large-w"
# python3 util/generate_rhg.py -n 2 -e 25 -k 256 --file_path "$SCRATCH/testing_graphs/strong" --file_name "large-dense"
# python3 util/generate_rhg.py -n 2 -e 25 -k 256 -w --file_path "$SCRATCH/testing_graphs/strong" --file_name "large-dense-w"

#weak scaling files
for exp in {16..24}; do
    python3 util/generate_rhg.py -n 2 -e $exp -k 64 --file_path "$SCRATCH/testing_graphs/weak" --file_name "2e$exp"
    python3 util/generate_rhg.py -n 2 -e $exp -k 64 -w --file_path "$SCRATCH/testing_graphs/weak" --file_name "2e$exp-w"
done