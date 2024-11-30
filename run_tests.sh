#!/bin/bash

# Compile the code
mpic++ v3.cpp -o v3

# Array sizes to test
sizes=(100 1000 10000)

# Number of processes to test (must be perfect squares)
processes=(1 4 9)

# Output file
output_file="ranking_performance.txt"
rm -f $output_file

for size in "${sizes[@]}"; do
    for procs in "${processes[@]}"; do
        echo "Running with size $size and $procs processes..."
        # Run 5 times and take average
        for i in {1..5}; do
            mpirun --oversubscribe -np $procs ./v3 $size >> $output_file
        done
    done
done
