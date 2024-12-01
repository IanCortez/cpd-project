#!/bin/bash

# Compile the code
mpic++ parallel_quicksort.cpp -o parallel_quicksort

# Array sizes to test
sizes=(10 100 1000 10000 100000 1000000)

# Number of processes to test (must be perfect squares)
processes=(1 4 9 25 36 49)

# Output file
output_file="quicksort_performance.txt"
rm -f $output_file

for size in "${sizes[@]}"; do
    for procs in "${processes[@]}"; do
        echo "Running with size $size and $procs processes..."
        # Run 5 times and take average
        for i in {1..5}; do
            # mpirun --oversubscribe -np $procs ./parallel_quicksort $size >> $output_file
            # mpirun -np $procs ./parallel_quicksort $size >> $output_file
            mpirun -np $procs ./parallel_quicksort $size
        done
    done
done
