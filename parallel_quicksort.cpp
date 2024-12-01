#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <fstream>
#include <chrono>

void sequential_quicksort(std::vector<int>& arr, int low, int high) {
    if (low >= high) return;
    
    int pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    int pivot_pos = i + 1;
    
    sequential_quicksort(arr, low, pivot_pos - 1);
    sequential_quicksort(arr, pivot_pos + 1, high);
}

std::vector<int> scatter_data(const std::vector<int>& data, int rank, int size) {
    int total_size = data.size();
    int local_size = total_size / size;
    
    std::vector<int> local_data(local_size);
    
    MPI_Scatter(data.data(), local_size, MPI_INT,
                local_data.data(), local_size, MPI_INT,
                0, MPI_COMM_WORLD);
    
    return local_data;
}

std::vector<int> gather_data(const std::vector<int>& local_data, int rank, int size, int total_size) {
    std::vector<int> gathered_data(total_size);
    
    MPI_Gather(local_data.data(), local_data.size(), MPI_INT,
               gathered_data.data(), local_data.size(), MPI_INT,
               0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        std::sort(gathered_data.begin(), gathered_data.end());
    }
    
    return gathered_data;
}

void write_performance_data(const std::string& filename, int array_size, int num_procs, double time_taken) {
    std::ofstream outfile;
    outfile.open(filename, std::ios_base::app); // Modo append
    outfile << array_size << "," << num_procs << "," << time_taken << "\n";
    outfile.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Diferentes tamaños de array para pruebas
    std::vector<int> array_sizes = {10, 100, 1000, 10000, 100000, 1000000};
    
    for (int ARRAY_SIZE : array_sizes) {
        // Ajustar tamaño para que sea divisible por el número de procesos
        // ARRAY_SIZE = ARRAY_SIZE - (ARRAY_SIZE % size);
        std::vector<int> data(ARRAY_SIZE);
        
        if (rank == 0) {
            // Inicializar datos
            srand(time(nullptr));
            for (int i = 0; i < ARRAY_SIZE; i++) {
                data[i] = rand() % 1000;
            }
        }
        
        // Sincronizar todos los procesos antes de empezar la medición
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();
        
        // Broadcast los datos iniciales
        MPI_Bcast(data.data(), ARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
        
        // Distribuir datos
        std::vector<int> local_data = scatter_data(data, rank, size);
        
        // Ordenar datos locales
        sequential_quicksort(local_data, 0, local_data.size() - 1);
        
        // Reunir datos ordenados
        std::vector<int> sorted_data = gather_data(local_data, rank, size, ARRAY_SIZE);
        
        // Sincronizar antes de medir el tiempo final
        MPI_Barrier(MPI_COMM_WORLD);
        double end_time = MPI_Wtime();
        double time_taken = end_time - start_time;
        
        // Solo el proceso 0 escribe los resultados
        if (rank == 0) {
            write_performance_data("quicksort_performance.txt", ARRAY_SIZE, size, time_taken);
            
            // Verificar si está ordenado
            // bool is_sorted = std::is_sorted(sorted_data.begin(), sorted_data.end());
            // std::cout << "Array size: " << ARRAY_SIZE << ", Correctly sorted: " 
            //           << (is_sorted ? "Yes" : "No") << ", Time: " << time_taken << " seconds" << std::endl;
        }
    }
    
    MPI_Finalize();
    return 0;
}