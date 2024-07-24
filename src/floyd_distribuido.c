#include "../include/floyd_distribuido.h"
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include <sys/time.h>

#define INF 9999 // Valor utilizado para representar la ausencia de conexion.
#define TAMANO_BLOQUE 2 // Tamanio de bloque para la programacion dinamica de OpenMP.

// Funcion para crear una matriz de adyacencia contigua en memoria.
int **crear_matriz_contigua(int cantidad_vertices) {
    int *datos = (int *)malloc(cantidad_vertices * cantidad_vertices * sizeof(int));
    int **distancias = (int **)malloc(cantidad_vertices * sizeof(int *));

    for (int i = 0; i < cantidad_vertices; i++) {
        distancias[i] = &(datos[cantidad_vertices * i]);
    }

    return distancias;
}

// Funcion para llenar la matriz de adyacencia desde un archivo.
int **readMatrixFromFile(char *ubicacion_matriz,int *cantidad_vertices) {
    const char *nombre_archivo = ubicacion_matriz;
    FILE *archivo = fopen(nombre_archivo, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    if (fscanf(archivo, "%d", cantidad_vertices) != 1) {
        fprintf(stderr, "Error al leer el numero de nodos desde el archivo\n");
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

// Funcion para mostrar la matriz de adyacencia.
void mostrar_matriz(int **distancias, int cantidad_vertices) {
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

// Funcion para mostrar la matriz de adyacencia.
void mostrar_filas_proceso(int **distancias, int inicio, int fin,int cantidad_vertices) {
    for (int i = inicio; i < fin ; i++) {
        for (int j = 0; j < cantidad_vertices; j++) {
            if (distancias[i][j] == INF)
                printf("%s\t", "INF");
            else
                printf("%d\t", distancias[i][j]);
        }
        printf("\n");
    }
}

// Funcion para liberar los recursos de la matriz de adyacencia.
void liberar_recursos(int **distancias) {
    free(distancias[0]); // Libera la memoria contigua de los datos de la matriz.
    free(distancias);    // Libera la memoria de los punteros a las filas.
}

// Funcion para abortar la ejecucion con un mensaje de error.
void abortar_con_error(int codigo_error) {
    if (codigo_error == NEGATIVE_NODE_NUMBER) {
        fprintf(stderr, "El numero de nodos debe ser positivo!\n");
    } else if (codigo_error == OUT_OF_MEMORY) {
        fprintf(stderr, "Memoria insuficiente!\n");
    } else if (codigo_error == STEP_LESS_THAN_ONE) {
        fprintf(stderr, "El numero de hilos no debe exceder el numero de nodos!\n");
    }
    abort();
}

int calcular_minimo(int a, int b) {
    if (a < b) return a;
    else return b;
}


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rango, num_procesos, cantidad_vertices;
    MPI_Comm_rank(MPI_COMM_WORLD, &rango);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

    printf("Comenzando, proceso numero %d\n", rango);

     // Verifica que se haya proporcionado un argumento para el numero de hilos.
    if (argc != 3) {
        if (rango == 0) { // Solo el proceso raiz debe imprimir el mensaje de error.
            printf( "Uso: <numero_de_hilos> <ubicacion_matriz>\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Finaliza todos los procesos.
    }
    char *ubicacion_matriz = argv[2];
    // Convierte el argumento de linea de comandos a un entero para obtener numHilos.
    int numHilos = atoi(argv[1]);
    if (numHilos <= 0) {
        if (rango == 0) { // Solo el proceso raiz debe imprimir el mensaje de error.
            fprintf(stderr, "El numero de hilos debe ser un entero positivo.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1); // Finaliza todos los procesos.
    }

    // Verificar que el numero de procesos sea adecuado.
    if (num_procesos == 1) {
        fprintf(stderr, "El numero de procesos no puede ser 1!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int **distancias;
    double tiempo_inicio, tiempo_final;

    if (rango == 0) {
        distancias = readMatrixFromFile(ubicacion_matriz, &cantidad_vertices);
        //mostrar_matriz(distancias, cantidad_vertices);
        printf("\n");
        tiempo_inicio = MPI_Wtime();
    }
    // Todos los procesos deben conocer la cantidad de vertices antes de recibir la matriz de distancias
    MPI_Bcast(&cantidad_vertices, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Inicializo la matriz en los escalvos.
    if(rango!=0){
            distancias = crear_matriz_contigua(cantidad_vertices);
    }
    // Transmitir la matriz completa, fila por fila, desde el proceso maestro a todos los esclavos.
    MPI_Bcast(distancias[0], cantidad_vertices * cantidad_vertices, MPI_INT, 0, MPI_COMM_WORLD);


    //paso: Es el numero de filas de la matriz que cada proceso va a computar.
    int paso = cantidad_vertices / num_procesos;
    
    //Primer Fila del proceso.
    int inicio = rango * paso;
    
    //Ultima fila del proceso.
    int fin = inicio + paso;

    if (rango == num_procesos - 1) {
        fin = cantidad_vertices;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Buffer para la fila k actualizada que se transmitir
    int *fila_k = (int *)malloc(cantidad_vertices * sizeof(int));
    // El proceso raiz (rango 0) prepara un bufer para recibir toda la matriz de distancias
    int *matriz_completa = NULL;
    if (rango == 0) {
        matriz_completa = (int *)malloc(cantidad_vertices * cantidad_vertices * sizeof(int));
    }
    
    //omp_set_num_threads(numHilos);
    for (int k = 0; k < cantidad_vertices; k++) {
        //La fila k la debe enviar el responsable
        int responsable=-1;
        if (k>=inicio && k<fin){
            responsable = rango;
        }
        // Garantiza que todos los procesos tengan el mismo valor de 'responsable'.
        MPI_Allreduce(MPI_IN_PLACE, &responsable, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        if (rango == responsable) {
            memcpy(fila_k, distancias[k], cantidad_vertices * sizeof(int));
        }
        //Envio fila k a todos los procesos.
        MPI_Bcast(fila_k, cantidad_vertices, MPI_INT, responsable, MPI_COMM_WORLD);

        // Cada proceso actualiza su porcion de la matriz
        //#pragma omp parallel for schedule(static) collapse(2)
        for (int i = inicio; i < fin; i++) {
            for (int j = 0; j < cantidad_vertices; j++) {
                if (distancias[i][k] == INF || fila_k[j] == INF) continue;
                distancias[i][j] = calcular_minimo(distancias[i][j], distancias[i][k] + fila_k[j]);
            }
        }
    }

    // Recolectar las porciones de la matriz de cada proceso en el proceso raiz
    MPI_Gather(distancias[inicio], paso * cantidad_vertices, MPI_INT, matriz_completa, paso * cantidad_vertices, MPI_INT, 0, MPI_COMM_WORLD);

    free(fila_k); // Se libera la memoria del buffer

    if (rango == 0) {
        tiempo_final = MPI_Wtime();
        printf("Matriz de distancias mas cortas entre cada par de vertices:\n");
        for (int i = 0; i < cantidad_vertices; i++) {
            for (int j = 0; j < cantidad_vertices; j++) {
                int valor = matriz_completa[i * cantidad_vertices + j];
                if (valor == INF) {
                    printf("INF\t");
                } else {
                    printf("%d\t", valor);
                }
            }
            printf("\n");
        }
        free(matriz_completa);
        printf("Tiempo transcurrido: %.6f segundos.\n", tiempo_final - tiempo_inicio);
    }
    liberar_recursos(distancias);
    MPI_Finalize();
    return 0;
}