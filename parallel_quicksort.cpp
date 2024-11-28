#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <climits>

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

// Función para dividir el array entre los procesos
std::vector<int> scatter_data(const std::vector<int>& data, int rank, int size) {
    int total_size = data.size();
    int local_size = total_size / size;
    
    std::vector<int> local_data(local_size);
    
    MPI_Scatter(data.data(), local_size, MPI_INT,
                local_data.data(), local_size, MPI_INT,
                0, MPI_COMM_WORLD);
    
    return local_data;
}

// Función para reunir los datos ordenados
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

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    const int ARRAY_SIZE = 20 - (20 % size);
    std::vector<int> data(ARRAY_SIZE);
    
    // Solo el proceso 0 genera los datos iniciales
    if (rank == 0) {
        srand(time(nullptr));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = rand() % 1000;
        }
        
        std::cout << "Array original:" << std::endl;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }
    
    // Broadcast the initial data to all processes
    MPI_Bcast(data.data(), ARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Distribuir datos entre procesos
    std::vector<int> local_data = scatter_data(data, rank, size);
    
    // Ordenar datos locales
    sequential_quicksort(local_data, 0, local_data.size() - 1);
    
    // Sincronizar procesos
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Reunir datos ordenados
    std::vector<int> sorted_data = gather_data(local_data, rank, size, ARRAY_SIZE);
    
    // Proceso 0 realiza la mezcla final y muestra resultados
    if (rank == 0) {
        std::cout << "\nArray ordenado:" << std::endl;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            std::cout << sorted_data[i] << " ";
        }
        std::cout << std::endl;
        
        // Verificar si está ordenado
        bool is_sorted = std::is_sorted(sorted_data.begin(), sorted_data.end());
        std::cout << "Array ordenado correctamente: " << (is_sorted ? "Sí" : "No") << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}