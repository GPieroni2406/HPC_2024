#include "floyd_distribuido_submatrices.h"
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include <math.h>
#include <omp.h>

MPI_Status status;

// Funciones definidas
int** allocMatrix(int rows) {
    int** matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(rows * sizeof(int));
    }
    return matrix;
}

void freeMatrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int readVerticesCount(FILE* file) {
    int vertices;
    if (fscanf(file, "%d", &vertices) != 1) {
        perror("Error al leer la cantidad de vértices");
        exit(EXIT_FAILURE);
    }
    return vertices;
}

void readMatrixFromFile(const char* filename, int* rows, int*** matrix) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    // Lee la primera fila para obtener la cantidad de vértices
    if (fscanf(file, "%d", rows) != 1) {
        perror("Error al leer la cantidad de vértices");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Asigna memoria para la matriz de adyacencia con la cantidad de vértices leída
    *matrix = (int**)malloc(*rows * sizeof(int*));
    for (int i = 0; i < *rows; i++) {
        (*matrix)[i] = (int*)malloc(*rows * sizeof(int));
    }

    // Llena la matriz con los datos del archivo
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *rows; j++) {
            if (fscanf(file, "%d", &(*matrix)[i][j]) != 1) {
                perror("Error al leer el archivo");
                // Libera la memoria asignada en caso de error
                for (int k = 0; k <= i; k++) {
                    free((*matrix)[k]);
                }
                free(*matrix);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}

// Esta función devuelve el mínimo de 2 números
int min(int a, int b) {
    if (a < b) return a;
    else return b;
}

// Esta función devuelve la suma de 2 números, manejando también los casos donde la entrada es infinito
int add(int a, int b) {
    if (a == INT_MAX && b == INT_MAX) return INT_MAX;
    if (a == INT_MAX || b == INT_MAX) return INT_MAX;

    return a + b;
}

int main(int argc, char **argv) {
    int rank, size,rows,numHilos;
    MPI_Init(&argc, &argv); // Inicia MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtiene el ID del proceso actual
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtiene el número de procesos
     // Verifica que se haya proporcionado un argumento para el número de hilos.
    if (argc != 2) {
        if (rank == 0) { // Solo el proceso raíz debe imprimir el mensaje de error.
            fprintf(stderr, "Uso: %s <numero_de_hilos>\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Finaliza todos los procesos.
    }

    // Convierte el argumento de línea de comandos a un entero para obtener numHilos.
    numHilos = atoi(argv[1]);
    if (numHilos <= 0) {
        if (rank == 0) { // Solo el proceso raíz debe imprimir el mensaje de error.
            fprintf(stderr, "El número de hilos debe ser un entero positivo.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Finaliza todos los procesos.
    }

    if (rank == 0) { // Inicio del código del proceso maestro
        // Proceso para dividir la gran matriz en fragmentos tipo grid
        int** originalArray_W0; // Inicializando la matriz original

        // Lee la matriz desde el archivo, que ahora también lee la cantidad de vértices
        readMatrixFromFile("input.txt", &rows, &originalArray_W0);

        int size_of_grid_chunk = rows / (sqrt((int)size)); // g = n / sqrt(p)

        int counter = 0;
        int i = 0;
        int j = 0;
        int k = 0;
        int g = size_of_grid_chunk; // g es el tamaño del fragmento tipo grid

        // Enviando los fragmentos de la matriz (grids) a los procesos esclavos
        for (i = 0; i < g; i++) {
            for (j = 0; j < g; j++) {
                for (k = 0; k < g; k++) {
                    MPI_Send(&originalArray_W0[(i * g) + k][j * g], g, MPI_INT, counter, k, MPI_COMM_WORLD);
                } // Fin del bucle k
                counter = counter + 1;
            } // Fin del bucle j
        } // Fin del bucle i

        // Recibiendo el fragmento del maestro (grid)
        int original_chunk[g][g];
        for (i = 0; i < g; i++) {
            MPI_Status status;
            MPI_Recv(&original_chunk[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
        }

        // Fin del proceso de dividir la gran matriz en fragmentos tipo grid

        // Bucle principal en el maestro que se ejecuta k veces donde k es el número de vértices en el grafo
        for (k = 0; k < rows; k++) {
            // Enviando las filas i
            for (i = 0; i < size; i++) {
                int i_by_g = i / g;
                for (j = i_by_g * g; j < (i_by_g * g) + g; j++) {
                    MPI_Send(&originalArray_W0[j][0], rows, MPI_INT, i, (j - (i_by_g * g)), MPI_COMM_WORLD); // Esto se ejecutará g veces para cada proceso
                } // Fin del bucle j
            } // Fin del bucle i

            // Recibiendo las filas i
            int i_rows_chunk[g][rows];
            for (i = 0; i < g; i++) {
                MPI_Status status;
                MPI_Recv(&i_rows_chunk[i][0], rows, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
            }

            // Enviando la fila k
            for (i = 0; i < size; i++) {
                MPI_Send(&originalArray_W0[k][0], rows, MPI_INT, i, k, MPI_COMM_WORLD); // la etiqueta es k
            }
            // Recibiendo la fila k
            int k_row[rows];
            MPI_Status status1;
            MPI_Recv(&k_row, rows, MPI_INT, 0, k, MPI_COMM_WORLD, &status1);

            // Inicia computación usando MULTI THREADING
            omp_set_num_threads(g);
            #pragma omp parallel for schedule(static, g/numHilos) collapse(2)
                for(int i=0 ; i<g ;i++){
                    for (int j = 0; j < g; j++) {
                        original_chunk[i][j] = min(original_chunk[i][j], add(i_rows_chunk[i][k], k_row[j]));
                    }
                }

            // Enviando los resultados al maestro para actualizar
            for (i = 0; i < g; i++) {
                MPI_Send(&original_chunk[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD);
            } // Fin del bucle i

            // Recibiendo de los procesos esclavos y actualizando la matriz original
            for (i = 0; i < size; i++) { // i representa el proceso del que recibimos
                int start_i = ((int)i / g) * g;
                int start_j = (i % g) * g;
                for (j = start_i; j < start_i + g; j++) {
                    MPI_Status status;
                    MPI_Recv(&originalArray_W0[j][start_j], g, MPI_INT, i, (start_i - j), MPI_COMM_WORLD, &status);
                } // Fin del bucle j
            } // Fin del bucle i
        } // Fin del bucle principal k

        printf("\nDespués de la computación, los valores del grid en el proceso %d son - \n", rank);

        for (i = 0; i < g; i++) {
            for (j = 0; j < g; j++) {
                printf(" %d ", original_chunk[i][j]);
            }
            printf("\n");
        }
        printf("\n\n");

        // Libera la memoria de la matriz original
        freeMatrix(originalArray_W0, rows);

    }     // Fin del código del proceso maestro

    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);//Envio filas a los esclavos

    if  (rank != 0 ) { // Proceso Esclavo
        int size_of_grid_chunk = rows / (sqrt((int)size)); // g = n / sqrt(p)
        int g = size_of_grid_chunk;

        // Recibiendo el fragmento del grid del maestro
        int original_chunk[g][g];
        for (int i = 0; i < g; i++) {
            MPI_Status status;
            MPI_Recv(&original_chunk[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
        }

        // Bucle principal en esclavo que se ejecuta k veces donde k es el número de vértices en el grafo
        for (int k = 0; k < rows; k++) {
            // Recibiendo las filas i del maestro
            int i_rows_chunk[g][rows];
            for (int i = 0; i < g; i++) {
                MPI_Status status;
                MPI_Recv(&i_rows_chunk[i][0], rows, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
            }

            // Recibiendo la fila k
            int k_row[rows];
            MPI_Status status1;
            MPI_Recv(&k_row, rows, MPI_INT, 0, k, MPI_COMM_WORLD, &status1);
            // Inicia computación usando MULTI THREADING
            omp_set_num_threads(g);
            #pragma omp parallel for schedule(static, g/numHilos) collapse(2)
                for(int i=0 ; i<g ;i++){
                    for (int j = 0; j < g; j++) {
                        original_chunk[i][j] = min(original_chunk[i][j], add(i_rows_chunk[i][k], k_row[j]));
                    }
                }
                
            // Enviando los resultados al maestro para actualizar
            for (int i = 0; i < g; i++) {
                MPI_Send(&original_chunk[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD);
            } // Fin del bucle i
        } // Fin del bucle principal k

        printf("\nDespués de la computación, los valores del grid en el proceso %d son - \n", rank);
        for (int i = 0; i < g; i++) {
            for (int j = 0; j < g; j++) {
                printf(" %d ", original_chunk[i][j]);
            }
            printf("\n");
        }
        printf("\n\n");
    } // Fin del Proceso Esclavo

    MPI_Finalize(); // Finaliza MPI
    return 0;
}