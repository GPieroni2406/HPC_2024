#include "floyd_distribuido.h"
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include <sys/time.h>

#define INF 9999 // Valor utilizado para representar la ausencia de conexión.
#define TAMANO_BLOQUE 2 // Tamaño de bloque para la programación dinámica de OpenMP.
//#define numHilos 4 // Número de hilos para OpenMP.

// Función para crear una matriz de adyacencia contigua en memoria.
int **crear_matriz_contigua(int cantidad_vertices) {
    int *datos = (int *)malloc(cantidad_vertices * cantidad_vertices * sizeof(int));
    int **distancias = (int **)malloc(cantidad_vertices * sizeof(int *));

    for (int i = 0; i < cantidad_vertices; i++) {
        distancias[i] = &(datos[cantidad_vertices * i]);
    }

    return distancias;
}

// Función para llenar la matriz de adyacencia desde un archivo.
int **llenar_matriz(int *cantidad_vertices) {
    const char *nombre_archivo = "input.txt";
    FILE *archivo = fopen(nombre_archivo, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    if (fscanf(archivo, "%d", cantidad_vertices) != 1) {
        fprintf(stderr, "Error al leer el número de nodos desde el archivo\n");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    int **matriz_adyacencia = crear_matriz_contigua(*cantidad_vertices);

    for (int i = 0; i < *cantidad_vertices; i++) {
        for (int j = 0; j < *cantidad_vertices; j++) {
            if (fscanf(archivo, "%d", &(matriz_adyacencia[i][j])) != 1) {
                fprintf(stderr, "Error al leer la matriz de adyacencia desde el archivo\n");
                free(matriz_adyacencia[0]);
                free(matriz_adyacencia);
                fclose(archivo);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(archivo);
    return matriz_adyacencia;
}

// Función para mostrar la matriz de adyacencia.
void mostrar_matriz(int **distancias, int cantidad_vertices) {
    printf("Matriz de distancias más cortas entre cada par de vértices:\n");
    for (int i = 0; i < cantidad_vertices; i++) {
        for (int j = 0; j < cantidad_vertices; j++) {
            if (distancias[i][j] == INF)
                printf("%s\t", "INF");
            else
                printf("%d\t", distancias[i][j]);
        }
        printf("\n");
    }
}

// Función para liberar los recursos de la matriz de adyacencia.
void liberar_recursos(int **distancias, int cantidad_vertices) {
    free(distancias[0]); // Libera el bloque de datos contiguos
    free(distancias);    // Libera el arreglo de punteros
}

// Función para abortar la ejecución con un mensaje de error.
void abortar_con_error(int codigo_error) {
    if (codigo_error == NEGATIVE_NODE_NUMBER) {
        fprintf(stderr, "El número de nodos debe ser positivo!\n");
    } else if (codigo_error == OUT_OF_MEMORY) {
        fprintf(stderr, "¡Memoria insuficiente!\n");
    } else if (codigo_error == STEP_LESS_THAN_ONE) {
        fprintf(stderr, "¡El número de hilos no debe exceder el número de nodos!\n");
    }
    abort();
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rango, num_procesos, cantidad_vertices;
    MPI_Comm_rank(MPI_COMM_WORLD, &rango);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

     // Verifica que se haya proporcionado un argumento para el número de hilos.
    if (argc != 2) {
        if (rango == 0) { // Solo el proceso raíz debe imprimir el mensaje de error.
            fprintf(stderr, "Uso: %s <numero_de_hilos>\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Finaliza todos los procesos.
    }

    // Convierte el argumento de línea de comandos a un entero para obtener numHilos.
    int numHilos = atoi(argv[1]);
    if (numHilos <= 0) {
        if (rango == 0) { // Solo el proceso raíz debe imprimir el mensaje de error.
            fprintf(stderr, "El número de hilos debe ser un entero positivo.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Finaliza todos los procesos.
    }

    // Verificar que el número de procesos sea adecuado.
    if (num_procesos == 1) {
        fprintf(stderr, "¡El número de procesos no puede ser 1!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int **distancias;
    double tiempo_inicio, tiempo_final;

    if (rango == 0) {
        distancias = llenar_matriz(&cantidad_vertices);
        mostrar_matriz(distancias, cantidad_vertices);
        tiempo_inicio = MPI_Wtime();
    }
    
    MPI_Bcast(*distancias, cantidad_vertices * cantidad_vertices, MPI_INT, 0, MPI_COMM_WORLD);

    int paso = cantidad_vertices / num_procesos;
    int inicio = rango * paso;
    int fin = inicio + paso;
    if (rango == num_procesos - 1) {
        fin = cantidad_vertices;
    }

    omp_set_num_threads(numHilos);
    for (int k = 0; k < cantidad_vertices; k++) {
        #pragma omp parallel for schedule(static, cantidad_vertices/numHilos) collapse(2)
        for (int i = inicio; i < fin; i++) {
            for (int j = 0; j < cantidad_vertices; j++) {
                if (distancias[i][k] == INF || distancias[k][j] == INF) continue;
                int nueva_distancia = distancias[i][k] + distancias[k][j];
                if (nueva_distancia < distancias[i][j]) {
                    distancias[i][j] = nueva_distancia;
                }
            }
        }
        MPI_Allreduce(MPI_IN_PLACE, *distancias, cantidad_vertices * cantidad_vertices, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    }

    if (rango == 0) {
        tiempo_final = MPI_Wtime();
        mostrar_matriz(distancias, cantidad_vertices);
        printf("Tiempo transcurrido: %.6f segundos.\n", tiempo_final - tiempo_inicio);
    }

    liberar_recursos(distancias, cantidad_vertices);
    MPI_Finalize();
    return 0;
}