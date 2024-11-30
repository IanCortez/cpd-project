#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// Versión 2: Implementación con estructura de grid
// - Agrega estructura de grid P x P
// - Implementa comunicación por filas y columnas
// - Agrega medición de tiempo
// - Mejora el manejo de memoria

class GridCommunicator {
private:
    int rank, size;
    int grid_dim;  // P en la estructura P x P
    int row, col;
    MPI_Comm row_comm;
    MPI_Comm col_comm;

public:
    GridCommunicator(MPI_Comm comm) {
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);
        
        // Calcular dimensión del grid
        grid_dim = static_cast<int>(sqrt(size));
        if (grid_dim * grid_dim != size) {
            if (rank == 0) {
                std::cerr << "Error: El número de procesos debe ser un cuadrado perfecto\n";
            }
            MPI_Abort(comm, 1);
        }

        // Calcular posición en el grid
        row = rank / grid_dim;
        col = rank % grid_dim;

        // Crear comunicadores para filas y columnas
        MPI_Comm_split(comm, row, col, &row_comm);
        MPI_Comm_split(comm, col, row, &col_comm);
    }

    ~GridCommunicator() {
        // Moved cleanup to explicit method
    }

    void cleanup() {
        MPI_Comm_free(&row_comm);
        MPI_Comm_free(&col_comm);
    }

    void broadcast_row(std::vector<int>& data) {
        MPI_Bcast(data.data(), data.size(), MPI_INT, 0, row_comm);
    }

    void broadcast_col(std::vector<int>& data) {
        MPI_Bcast(data.data(), data.size(), MPI_INT, 0, col_comm);
    }

    int get_rank() const { return rank; }
    int get_row() const { return row; }
    int get_col() const { return col; }
    int get_grid_dim() const { return grid_dim; }
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    // Crear comunicador de grid
    GridCommunicator grid(MPI_COMM_WORLD);
    const int N = 16;
    
    // Inicio de medición de tiempo
    double start_time = MPI_Wtime();

    std::vector<int> data(N);
    if (grid.get_rank() == 0) {
        std::srand(std::time(0));
        for(int i = 0; i < N; i++) {
            data[i] = std::rand() % 100;
            std::cout << "Original[" << i << "] = " << data[i] << std::endl;
        }
    }

    // Distribuir datos usando la estructura de grid
    MPI_Bcast(data.data(), N, MPI_INT, 0, MPI_COMM_WORLD);

    // Calcular tamaño local basado en la posición en el grid
    int P = grid.get_grid_dim();
    int local_size = N / (P * P);
    int start = grid.get_rank() * local_size;
    int end = (grid.get_rank() == P*P-1) ? N : start + local_size;

    // Cálculo de rankings locales
    std::vector<int> local_ranks(N, 0);  
    for(int i = start; i < end; i++) {
        for(int j = 0; j < N; j++) {
            if(data[i] > data[j] || (data[i] == data[j] && i > j)) {
                local_ranks[i]++;
            }
        }
    }

    // Reducir los rankings usando MPI_Allreduce
    std::vector<int> global_ranks(N);
    MPI_Allreduce(local_ranks.data(), global_ranks.data(), N, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Recolectar resultados
    if(grid.get_rank() == 0) {
        std::vector<int> result(N);
        for(int i = 0; i < N; i++) {
            result[global_ranks[i]] = data[i];
        }

        std::cout << "\nResultado ordenado:" << std::endl;
        for(int i = 0; i < N; i++) {
            std::cout << "Sorted[" << i << "] = " << result[i] << std::endl;
        }

        std::cout << "Tiempo de ejecución: " << MPI_Wtime() - start_time << " segundos" << std::endl;
    }

    grid.cleanup();  // Clean up MPI communicators before finalizing
    MPI_Finalize();
    return 0;
}