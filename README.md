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

## generate random hyperbolic graphs
```bash
source modules.sh
conda env create -f conda_env.yaml
conda activate capforest-env
```