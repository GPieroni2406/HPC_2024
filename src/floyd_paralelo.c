#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include "../include/floyd_paralelo.h"

// Funcion para leer la matriz de adyacencia de un grafo desde un archivo.
int **leerGrafoDesdeArchivo(const char *nombreArchivo, int *cantidadVertices) {
    FILE *archivoEntrada = fopen(nombreArchivo, "r");
    if (archivoEntrada == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    fscanf(archivoEntrada, "%d", cantidadVertices);
    int **grafo = (int **)malloc(*cantidadVertices * sizeof(int *));
    for (int i = 0; i < *cantidadVertices; i++) {
        grafo[i] = (int *)malloc(*cantidadVertices * sizeof(int));
        for (int j = 0; j < *cantidadVertices; j++) {
            fscanf(archivoEntrada, "%d", &grafo[i][j]);
        }
    }
    fclose(archivoEntrada);
    return grafo;
}

// Funcion que implementa el algoritmo de Floyd-Warshall paralelizado con OpenMP.
void algoritmoFloydWarshallParalelo(int **grafo, int cantidadVertices, int numHilos) {
    int **matrizDistancias = (int **)malloc(cantidadVertices * sizeof(int *));
    for (int i = 0; i < cantidadVertices; i++) {
        matrizDistancias[i] = (int *)malloc(cantidadVertices * sizeof(int));
        for (int j = 0; j < cantidadVertices; j++) {
            matrizDistancias[i][j] = grafo[i][j];
        }
    }

    // Paraleliza el algoritmo de Floyd-Warshall utilizando OpenMP.
    for (int k = 0; k < cantidadVertices; k++) {
        #pragma omp parallel for schedule(static) collapse(2) num_threads(numHilos)
        for (int i = 0; i < cantidadVertices; i++) {
            for (int j = 0; j < cantidadVertices; j++) {
                if (matrizDistancias[i][k] + matrizDistancias[k][j] < matrizDistancias[i][j]) {
                    matrizDistancias[i][j] = matrizDistancias[i][k] + matrizDistancias[k][j];
                }
            }
        }
    }
    
    mostrarMatrizDeDistancias(matrizDistancias, cantidadVertices, numHilos);

    for (int i = 0; i < cantidadVertices; i++) {
        free(matrizDistancias[i]);
    }
    free(matrizDistancias);
}

// Funcion para imprimir la matriz de distancias mas cortas entre cada par de vertices..
void mostrarMatrizDeDistancias(int **matrizDistancias, int cantidadVertices, int numHilos) {
    printf("La siguiente matriz muestra las distancias m�s cortas entre cada par de v�rtices\n");
    for (int i = 0; i < cantidadVertices; i++) {
        for (int j = 0; j < cantidadVertices; j++) {
            if (matrizDistancias[i][j] == DIS_INFINITA)
                printf("%s   ", "DIS_INFINITA");
            else
                printf("%d     ", matrizDistancias[i][j]);
        }
        printf("\n");
    }
}

// La funcion principal del programa.
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: ./floyd_paralelo <numero_de_hilos> <ubicacion de matriz> \n");
        exit(EXIT_FAILURE);
    }
    int numHilos = atoi(argv[1]);
    char *ubicacion_matriz = argv[2];
    if (numHilos <= 0) {
        fprintf(stderr, "El numero de hilos debe ser un entero positivo.\n");
        exit(EXIT_FAILURE);
    }
    int cantidadVertices;
    int **grafo = leerGrafoDesdeArchivo(ubicacion_matriz, &cantidadVertices);
    if (grafo == NULL) {
        return 1;
    }

    struct timeval tiempoInicio;
    struct timezone zonaHorariaInicio;
    struct timeval tiempoFinal;
    struct timezone zonaHorariaFinal;
    long tiempoInicioMicroseg, tiempoFinalMicroseg;
    double tiempoTotal;

    omp_set_num_threads(numHilos);

    gettimeofday(&tiempoInicio, &zonaHorariaInicio);

    algoritmoFloydWarshallParalelo(grafo, cantidadVertices, numHilos);

    gettimeofday(&tiempoFinal, &zonaHorariaFinal);

    tiempoInicioMicroseg = tiempoInicio.tv_sec * 1000000 + tiempoInicio.tv_usec;
    tiempoFinalMicroseg = tiempoFinal.tv_sec * 1000000 + tiempoFinal.tv_usec;
    tiempoTotal = (tiempoFinalMicroseg - tiempoInicioMicroseg) / 1000000.0;
    printf("\nTiempo en segundos (t): %lf\n", tiempoTotal);

    for (int i = 0; i < cantidadVertices; i++) {
        free(grafo[i]);
    }
    free(grafo);

    return 0;
}