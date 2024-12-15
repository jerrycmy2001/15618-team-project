#!/bin/bash
module load cuda

scenes=(
    # "single"
    # "double"
    # "non-orthogonal"
    # "square"
    # "cube"
    # "tetrahedron"
    "rand216"
    "rand343"
    "rand512"
    "rand729"
    "rand1000"
)

index=0

# While loop to iterate through the list
while [ $index -lt ${#scenes[@]} ]; do
    scene=${scenes[$index]}

    # echo "===== Scene $scene CPU ====="
    # ./render $scene -b 0:1

    echo "===== Scene $scene GPU ====="
    ./render $scene -b 0:1 -r cuda

    ((index++))
done
