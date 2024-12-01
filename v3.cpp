#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Función para validar que p es un cuadrado perfecto
bool esCuadradoPerfecto(int p) {
    int raiz = std::sqrt(p);
    return (raiz * raiz == p);
}

//QuickSort implementation
void quickSort(std::vector<char>& arr, int low, int high) {
    if (low < high) {
        //Choose the rightmost element as pivot
        char pivot = arr[high];
        
        //Index of smaller element and indicates
        //the right position of pivot found so far
        int i = (low - 1);
        
        for (int j = low; j <= high - 1; j++) {
            //If current element is smaller than or equal to pivot
            if (arr[j] <= pivot) {
                //Increment index of smaller element
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        int partitionIndex = i + 1;
        
        //Separately sort elements before and after partition
        quickSort(arr, low, partitionIndex - 1);
        quickSort(arr, partitionIndex + 1, high);
    }
}

//Binary search to find the number of elements less than or equal to a given element
int binarySearchRank(const std::vector<char>& sortedArr, char target) {
    int left = 0;
    int right = sortedArr.size() - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (sortedArr[mid] <= target) {
            //If the middle element is less than or equal, 
            //check if this is the last such element
            if (mid == sortedArr.size() - 1 || sortedArr[mid + 1] > target) {
                return mid + 1;  //Return the rank (1-based index)
            }
            left = mid + 1;  //Look in the right half
        } else {
            right = mid - 1;  //Look in the left half
        }
    }
    
    return 0;  //No elements less than or equal to target
}



int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get array size from command line
    if (argc != 2) {
        if (rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <array_size>\n";
        }
        MPI_Finalize();
        return -1;
    }
    int n = std::stoi(argv[1]);

    if (!esCuadradoPerfecto(size)) {
        if (rank == 0) {
            std::cerr << "Error: El número de procesos debe ser un cuadrado perfecto.\n";
        }
        MPI_Finalize();
        return -1;
    }

    // Start timing
    double start_time = MPI_Wtime();

    //Generamos valores aleatorios en el proceso 0
    std::vector<char> datos_totales;

    if (rank == 0) {
        std::srand(std::time(0));
        for (int i = 0; i < n; ++i) {
            datos_totales.push_back('a' + (std::rand() % 26));
        }
        //std::cout << "Datos iniciales: ";
        //for (char c : datos_totales) std::cout << c << " ";
        //std::cout << std::endl;
    }


    /*
    ====================================================================
    ====================================================================
                            Paso 1: Input
    ====================================================================
    ====================================================================
    */

    //std::vector<char> datos_totales; -> Datos aleatorios generados en el proceso 0

    int elementos_por_proceso = n / size; //Número de elementos que cada proceso recibirá
    int raiz_p = std::sqrt(size); //Número de elementos por fila o columna
    int fila = rank / raiz_p; //Fila a la que pertenece el proceso
    int columna = rank % raiz_p; //Columna a la que pertenece el proceso

    std::vector<char> datos_locales(elementos_por_proceso);

    //Distribuir los datos entre los procesos
    MPI_Scatter(datos_totales.data(), elementos_por_proceso, MPI_CHAR,
                datos_locales.data(), elementos_por_proceso, MPI_CHAR, 0, MPI_COMM_WORLD);

    // std::cout << "Proceso " << rank << " de (fila, columna): ("<< fila << "," << columna << ") " << " recibió: ";
    // for (char c : datos_locales) std::cout << c << " ";
    // std::cout << std::endl;

    /*
    ====================================================================
    ====================================================================
                            Paso 2: Gossip
    ====================================================================
    ====================================================================
    */

    //int raiz_p = std::sqrt(size); //Número de elementos por fila o columna
    //int fila = rank / raiz_p; //Fila a la que pertenece el proceso
    //int columna = rank % raiz_p; //Columna a la que pertenece el proceso

    
    //Crear un comunicador para cada columna
    MPI_Comm columna_comm;
    int color = columna; //Los procesos con el mismo número de columna tendrán el mismo color
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &columna_comm);

    //Obtener el nuevo rank y size dentro del comunicador de la columna
    int columna_rank, columna_size;
    MPI_Comm_rank(columna_comm, &columna_rank);
    MPI_Comm_size(columna_comm, &columna_size);

    //Vector para almacenar todos los datos de la columna
    std::vector<char> datos_columna(elementos_por_proceso * columna_size);

    //Reunir todos los datos de la columna en cada proceso de la columna
    MPI_Allgather(datos_locales.data(), elementos_por_proceso, MPI_CHAR,
                  datos_columna.data(), elementos_por_proceso, MPI_CHAR,
                  columna_comm);

    //Mostrar los datos reunidos en cada proceso
    // std::cout << "Proceso " << rank << " tiene después del Gossip: ";
    // for (char c : datos_columna) std::cout << c << " ";
    // std::cout << std::endl;

    //Liberar el comunicador de columna
    MPI_Comm_free(&columna_comm);

    /*
    ====================================================================
                            Paso 3: Broadcast
    ====================================================================
    */

    //int raiz_p = std::sqrt(size); //Número de elementos por fila o columna
    //int fila = rank / raiz_p; //Fila a la que pertenece el proceso
    //int columna = rank % raiz_p; //Columna a la que pertenece el proceso

    //Mantenemos las variables existentes
    //int raiz_p = std::sqrt(size);
    //int fila = rank / raiz_p;
    //int columna = rank % raiz_p;

    //Crear comunicador para cada fila (mantenemos esto igual)
    MPI_Comm fila_comm;
    MPI_Comm_split(MPI_COMM_WORLD, fila, rank, &fila_comm);

    //Vector para los datos recibidos
    std::vector<char> datos_recibidos(elementos_por_proceso * columna_size);


    bool es_diagonal = (fila == columna);

    //Si es proceso diagonal, copia sus datos_columna
    if (es_diagonal) {
        datos_recibidos = datos_columna;
    }

    //Broadcast desde el proceso diagonal (columna == fila) a todos en su fila
    MPI_Bcast(datos_recibidos.data(), 
            elementos_por_proceso * columna_size, 
            MPI_CHAR, 
            fila,  // El proceso diagonal tiene el mismo número de columna que de fila
            fila_comm);


    // Opcional: Verificación de datos
    // std::cout << "Proceso " << rank << " (fila=" << fila << ", col=" << columna 
    //           << ") tiene después del Broadcast optimizado: ";
    // for (char c : datos_recibidos) std::cout << c << " ";
    // std::cout << std::endl;

    //MPI_Comm_free(&fila_comm); -> En paso 6 recien lo ponemos por que vamos a volver a operar en filas

    /*
    ====================================================================
                            Paso 4: Sorting
    ====================================================================
    */
    //datos_columna -> Datos despues del Gossip


    //Perform QuickSort on the received data
    quickSort(datos_columna, 0, datos_columna.size() - 1);

    // std::cout << "Proceso " << rank << " después de Sorting: ";
    // for (char c : datos_columna) std::cout << c << " ";
    // std::cout << std::endl;

    /*
    ====================================================================
                            Paso 5: Local Ranking
    ====================================================================
    */
    
    //datos_recibidos are the distributed elements
    //datos_columna is the sorted column data
    
    //Vector to store local rankings
    std::vector<int> local_ranking(datos_recibidos.size());
    
    //Compute local ranking for each element using binary search
    for (size_t i = 0; i < datos_recibidos.size(); ++i) {
        local_ranking[i] = binarySearchRank(datos_columna, datos_recibidos[i]);
    }

    //Print local ranking
    // std::cout << "Proceso " << rank << " Local Ranking: ";
    // for (int ranking : local_ranking) {
    //     std::cout << ranking << " ";
    // }
    // std::cout << std::endl;

    /*
    ====================================================================
                            Paso 6: Global Ranking / Reduce
    ====================================================================
    */
    //int raiz_p = std::sqrt(size); //Número de elementos por fila o columna
    //int fila = rank / raiz_p; //Fila a la que pertenece el proceso
    //int columna = rank % raiz_p; //Columna a la que pertenece el proceso
    //std::vector<int> local_ranking; -> Rankings locales calculados en el paso 5
    //bool es_diagonal = (fila == columna); -> Verificar si este proceso está en la diagonal
    /*
    Ademas se utiliza el comunicador de fila creado en el paso 3
    // MPI_Comm fila_comm;
    // int color = fila;
    // MPI_Comm_split(MPI_COMM_WORLD, color, rank, &fila_comm);

    // int fila_rank, fila_size;
    // MPI_Comm_rank(fila_comm, &fila_rank);
    // MPI_Comm_size(fila_comm, &fila_size);
    */


    //Vector para almacenar el resultado de la reducción
    std::vector<int> global_ranking(local_ranking.size());

    //Proceso raíz de cada fila (el de la diagonal)
    int root = fila;

    //Realizar la reducción de suma de rankings locales
    MPI_Reduce(local_ranking.data(),  //Datos de envío
               es_diagonal ? global_ranking.data() : NULL,  //Buffer de recepción (solo en diagonal)
               local_ranking.size(),  //Número de elementos
               MPI_INT,               //Tipo de dato
               MPI_SUM,               //Operación de reducción (suma)
               root,                  //Proceso raíz
               fila_comm);            //Comunicador de fila

    //Mostrar el ranking global para los procesos en la diagonal
    // if (es_diagonal) {
    //     std::cout << "Proceso " << rank << " Ranking Global: ";
    //     for (int r : global_ranking) {
    //         std::cout << r << " ";
    //     }
    //     std::cout << std::endl;
    // }

    MPI_Comm_free(&fila_comm);
    

    // End timing
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        std::cout << n << "," << size << "," << (end_time - start_time) << std::endl;
    }

    MPI_Finalize();
    return 0;
}