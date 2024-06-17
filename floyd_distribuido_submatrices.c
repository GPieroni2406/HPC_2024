#include "floyd_distribuido_submatrices.h"
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include <math.h>
#include <omp.h>

MPI_Status status;

void freeMatrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int** readMatrixFromFile(const char* filename, int* rows) {
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

    int ** matrix = (int **)malloc(*rows * sizeof(int *));
    for (int i = 0; i < *rows; i++) {
        matrix[i] = (int *)malloc(*rows * sizeof(int));
        for (int j = 0; j < *rows; j++) {
            // Lee la matriz de adyacencia del grafo desde el archivo.
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
    fclose(file);
    return matrix;
}

// Esta función devuelve el mínimo de 2 números
int calcular_minimo(int a, int b) {
    if (a < b) return a;
    else return b;
}

// Esta función devuelve la suma de 2 números, manejando también los casos donde la entrada es infinito
int suma(int a, int b) {
    if (a == INT_MAX && b == INT_MAX) return INT_MAX;
    if (a == INT_MAX || b == INT_MAX) return INT_MAX;

    return a + b;
}

int main(int argc, char **argv) {
    int rank, size,rows,numHilos;
    int** matriz_original;
    MPI_Init(&argc, &argv); // Inicia MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtiene el ID del proceso actual
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtiene el número de procesos

    // Comprueba si 'size' es un cuadrado perfecto
    double sqrt_size = sqrt((double)size);
    if ((int)(sqrt_size + 0.5) * (int)(sqrt_size + 0.5) != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: El número de procesos (%d) debe ser un cuadrado perfecto.\n", size);
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Termina todos los procesos con un código de error.
    }
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

    printf("Comenzando, proceso numero %d\n", rank);

    if (rank == 0) { // Inicio del código del proceso maestro
        // Proceso para dividir la gran matriz en fragmentos tipo grid
        matriz_original = readMatrixFromFile("input.txt", &rows); ; // Inicializando la matriz original
    }
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);//Envio filas a los esclavos
    if (rank == 0){
        printf("cantidad de rows enviadas a los esclavos\n");
    }

        // Comprueba si 'rows' es divisible por la raíz cuadrada de 'size'
    int sqrt_size_ = (int)sqrt((double)size);
    if (rows % sqrt_size_ != 0) {
        if (rank == 0){
            fprintf(stderr, "Error: El número de filas (%d) no es divisible por la raíz cuadrada del número de procesos (%d).\n", rows, sqrt_size_);
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Termina todos los procesos con un código de error.
    }

    if (rank==0){
        int tamanio_submatriz = rows / (sqrt((int)size)); // g = n / sqrt(p)
        int counter = 0;
        int i = 0;
        int j = 0;
        int k = 0;
        int g = tamanio_submatriz; // g es el tamaño del fragmento tipo grid
        // Enviando los fragmentos de la matriz (grids) a los procesos esclavos
        for (i = 0; i < g; i++) {
            for (j = 0; j < g; j++) {
                for (k = 0; k < g; k++) {
                    MPI_Send(&matriz_original[(i * g) + k][j * g], g, MPI_INT, counter, k, MPI_COMM_WORLD);
                } // Fin del bucle k

                // Imprimir las partes de la matriz que están siendo enviadas
                printf("Enviando partes de la matriz al proceso %d:\n", counter);
                for (k = 0; k < g; k++) {
                    // Imprime la fila completa que está siendo enviada
                    for (int l = 0; l < g; l++) {
                        printf("%d ", matriz_original[(i * g) + k][(j * g) + l]);
                    }
                    printf("\n");
                }
                counter = counter + 1;
            } // Fin del bucle j
        } // Fin del bucle i

        // Recibiendo el fragmento del maestro (grid)
        int matriz_actualizada[g][g];
        for (i = 0; i < g; i++) {
            MPI_Status status;
            MPI_Recv(&matriz_actualizada[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
        }
            //IMPRIMO LA MATRIZ QUE RECIBO
        printf("Proceso %d recibió la siguiente submatriz:\n", rank);
        for (i = 0; i < g; i++) {
            for (j = 0; j < g; j++) {
                printf("%d ", matriz_actualizada[i][j]);
            }
            printf("\n");
        }

        // Fin del proceso de dividir la gran matriz en fragmentos tipo grid

        // Bucle principal en el maestro que se ejecuta k veces donde k es el número de vértices en el grafo
        for (k = 0; k < rows; k++) {
            // Enviando las filas i
            for (i = 0; i < size; i++) {
                int i_by_g = i / g;
                for (j = i_by_g * g; j < (i_by_g * g) + g; j++) {
                    MPI_Send(&matriz_original[j][0], rows, MPI_INT, i, (j - (i_by_g * g)), MPI_COMM_WORLD); // Esto se ejecutará g veces para cada proceso
                } // Fin del bucle j
            } // Fin del bucle i

            // Recibiendo las filas i
            int i_filas_act[g][rows];
            for (i = 0; i < g; i++) {
                MPI_Status status;
                MPI_Recv(&i_filas_act[i][0], rows, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
            }

            // Enviando la fila k
            for (i = 0; i < size; i++) {
                MPI_Send(&matriz_original[k][0], rows, MPI_INT, i, k, MPI_COMM_WORLD); // la etiqueta es k
            }

            // Recibiendo la fila k
            int k_row[rows];
            MPI_Status status1;
            MPI_Recv(&k_row, rows, MPI_INT, 0, k, MPI_COMM_WORLD, &status1);

            omp_set_num_threads(numHilos); // Configura el número de hilos para OpenMP
            #pragma omp parallel // Inicia un bloque paralelo
            {
                // Obtiene el ID del hilo dentro del bloque paralelo
                int id = omp_get_thread_num();

                // Realiza la actualización en paralelo
                for (int j = 0; j < g; j++) {
                    //int absolute_i = (((int)rank / g) * g) + id;
                    int pos_j = ((rank % g) * g) + j;
                    
                    // Asegúrate de que el acceso a matriz_actualizada, i_filas_act y k_row es correcto
                    matriz_actualizada[id][j] = calcular_minimo(matriz_actualizada[id][j], suma(i_filas_act[id][k], k_row[pos_j]));
                }
            }
            // Enviando los resultados al maestro para actualizar
            for (i = 0; i < g; i++) {
                MPI_Send(&matriz_actualizada[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD);
            } // Fin del bucle i

            // Recibiendo de los procesos esclavos y actualizando la matriz original
            for (i = 0; i < size; i++) { // i representa el proceso del que recibimos
                int start_i = ((int)i / g) * g;
                int start_j = (i % g) * g;
                for (j = start_i; j < start_i + g; j++) {
                    MPI_Status status;
                    MPI_Recv(&matriz_original[j][start_j], g, MPI_INT, i, (start_i - j), MPI_COMM_WORLD, &status);
                } // Fin del bucle j
            } // Fin del bucle i
        } // Fin del bucle principal k

        printf("\nDespués de la computación, los valores del grid en el proceso %d son - \n", rank);

        for (i = 0; i < g; i++) {
            for (j = 0; j < g; j++) {
                printf(" %d ", matriz_actualizada[i][j]);
            }
            printf("\n");
        }
        printf("\n\n");

    }     // Fin del código del proceso maestro

    if  (rank != 0 ) { // Proceso Esclavo
        int tamanio_submatriz = rows / (sqrt((int)size)); // g = n / sqrt(p)
        int g = tamanio_submatriz;

        // Recibiendo el fragmento del grid del maestro
        int matriz_actualizada[g][g];
        for (int i = 0; i < g; i++) {
            MPI_Status status;
            MPI_Recv(&matriz_actualizada[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
        }
                // Imprimiendo la matriz que se recibió
        printf("Proceso %d recibió la siguiente submatriz:\n", rank);
        for (int i = 0; i < g; i++) {
            for (int j = 0; j < g; j++) {
                printf("%d ", matriz_actualizada[i][j]);
            }
            printf("\n");
        }

        // Bucle principal en esclavo que se ejecuta k veces donde k es el número de vértices en el grafo
        for (int k = 0; k < rows; k++) {
            // Recibiendo las filas i del maestro
            int i_filas_act[g][rows];
            for (int i = 0; i < g; i++) {
                MPI_Status status;
                MPI_Recv(&i_filas_act[i][0], rows, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
            }
            // Recibiendo la fila k
            int k_row[rows];
            MPI_Status status1;
            MPI_Recv(&k_row, rows, MPI_INT, 0, k, MPI_COMM_WORLD, &status1);

            omp_set_num_threads(numHilos); // Configura el número de hilos para OpenMP
            #pragma omp parallel // Inicia un bloque paralelo
            {
                // Obtiene el ID del hilo dentro del bloque paralelo
                int id = omp_get_thread_num();

                // Realiza la actualización en paralelo
                for (int j = 0; j < g; j++) {
                    //int absolute_i = (((int)rank / g) * g) + id;
                    int pos_j = ((rank % g) * g) + j;
                    
                    // Asegúrate de que el acceso a matriz_actualizada, i_filas_act y k_row es correcto
                    matriz_actualizada[id][j] = calcular_minimo(matriz_actualizada[id][j], suma(i_filas_act[id][k], k_row[pos_j]));
                }
            }
            // Enviando los resultados al maestro para actualizar
            for (int i = 0; i < g; i++) {
                MPI_Send(&matriz_actualizada[i][0], g, MPI_INT, 0, i, MPI_COMM_WORLD);
            } // Fin del bucle i
        } // Fin del bucle principal k

        printf("\nDespués de la computación, los valores del grid en el proceso %d son - \n", rank);
        for (int i = 0; i < g; i++) {
            for (int j = 0; j < g; j++) {
                printf(" %d ", matriz_actualizada[i][j]);
            }
            printf("\n");
        }
        printf("\n\n");
    } // Fin del Proceso Esclavo
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0){
        printf("Esta es la matriz luego del algoritmo!\n");
        for (int n = 0; n < rows; n++) {
            for (int m = 0; m < rows; m++) {
                if(matriz_original[n][m]==9999){
                    printf(" %s ", "Infinito");
                }
                else{
                    printf(" %d ", matriz_original[n][m]);
                }
            }
            printf("\n");
        }
        // Libera la memoria de la matriz original
        freeMatrix(matriz_original, rows);
    }

    MPI_Finalize(); // Finaliza MPI
    return 0;
}