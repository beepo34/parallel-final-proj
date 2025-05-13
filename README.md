# Parallel Final Project
Aileen Mi, Alexandra Zhang Jiang, Calix Tang

## build
```bash
source modules.sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=CC ..
make
```

## run
```
cd build
salloc -N {num_nodes} -A mp309 -t 10:00 --qos=interactive -C cpu srun -N {num_nodes} --ntasks-per-node {tasks_per_node} ./mincut {path_to_metis_graph_file}
```

## generate random hyperbolic graphs
```bash
source modules.sh
conda env create -f conda_env.yaml
conda activate capforest-env
```
