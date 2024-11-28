quicksort:
	rm -f parallel_quicksort; mpic++ -o parallel_quicksort parallel_quicksort.cpp && mpirun -np 4 ./parallel_quicksort