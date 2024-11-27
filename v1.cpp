#include <mpi.h>
#include <iostream>
#include <cmath>
using namespace std;


int determineRank(int elemento, int* lista, int size) {
    int ranking = 0;
    for (int i=0; i<size; ++i) {
        if (lista[i] < elemento) {
            ranking++;
        }
    }
    return ranking;
}


// p = P * P
// Ordenamiento en paralelo basico, no sigue por completo la estructura mostrada
int main(int argc, char** argv) {
    int rank, size;
    int N = 16;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Generar datos
    int data[N];
    if (rank == 0) {
        for (int i=0; i<N; ++i) {
            data[i] = std::rand();
        }
    }
    
    // Conseguir cantidad de procesos
    int P = (int) std::sqrt(size);

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Partir el arreglo de datos entre los P*P procesos
    MPI_Bcast(data, N, MPI_INT, 0, MPI_COMM_WORLD);

    // Definir
    int local_n = (N + size - 1) / size;
    int start = rank * local_n;
    int end = std::min(start + local_n, N);

    int* local_order = new int[end-start];

    for (int i=start; i<end; ++i) {
        local_order[i - start] = determineRank(data[i], data, N);
    }

    int* rankings = new int[N];
    MPI_Gather(local_order, end-start, MPI_INT, rankings, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int* ordered_data = new int[N];
        for (int i=0; i<N; ++i) {
            ordered_data[rankings[i]] = data[i];
        }

        std::cout << "Ordered list:" << std::endl;
        for (int i=0; i<N; ++i) {
            std::cout << ordered_data[i] << ' ';
        }
        std::cout << std::endl;
    }

    delete[] rankings;
    delete[] local_order;
    MPI_Finalize();

    return 0;
}