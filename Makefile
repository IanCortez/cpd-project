quicksort:
	rm -f parallel_quicksort; mpic++ -std=c++11 -o parallel_quicksort parallel_quicksort.cpp && mpirun -np 4 ./parallel_quicksort