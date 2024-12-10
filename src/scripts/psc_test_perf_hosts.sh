#!/bin/bash
module load cuda
module load openmpi
find /jet/packages/cuda -name 'libcudart.so.11.0'
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/jet/packages/cuda/v11.6.0/targets/x86_64-linux/lib/

scenes=(
    "single"
    "double"
    "non-orthogonal"
    "square"
    "cube"
    "tetrahedron"
    "rand"
)

index=0

# While loop to iterate through the list
while [ $index -lt ${#scenes[@]} ]; do
    scene=${scenes[$index]}

    scontrol show hostnames "$SLURM_JOB_NODELIST"
    nodes=$(scontrol show hostnames "$SLURM_JOB_NODELIST" | sed 's/$/:2/' | tr '\n' ',' | sed 's/,$//')
    echo "===== Scene $scene GPUx2 $nodes ====="
    mpirun -np 2 --host $nodes ./render $scene -b 0:1 -r cuda -p 2 -a 2

    ((index++))
done
