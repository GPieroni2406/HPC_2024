#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define INF 9999
#define N 512  // Cambia esto según el tamaño de tu matriz

void floydWarshall(int graph[N][N]) {
    int dist[N][N], i, j, k;

    // Inicializar la matriz de distancias
    #pragma omp parallel for collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            dist[i][j] = graph[i][j];
        }
    }

    double start_time = omp_get_wtime();

    // Aplicar el algoritmo de Floyd-Warshall
    for (k = 0; k < N; k++) {
        #pragma omp parallel for collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    double end_time = omp_get_wtime();

    // Imprimir la matriz de distancias
    /*printf("La siguiente matriz muestra las distancias más cortas entre cada par de vértices\n");
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (dist[i][j] == INF)
                printf("%7s", "INF");
            else
                printf("%7d", dist[i][j]);
        }
        printf("\n");
    } */



 
    // Imprimir el tiempo de ejecución
    printf("Tiempo de ejecución: %f segundos\n", end_time - start_time);

    // Imprimir el número de hilos usados
    #pragma omp parallel
    {
        #pragma omp master
        {
            printf("Número de hilos utilizados: %d\n", omp_get_num_threads());
        }
    }
}

int main() {
    int graph[N][N];
    
    // Inicializar la matriz de adyacencia con valores de ejemplo
    // Puedes cargar la matriz desde un archivo o inicializarla manualmente aquí
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) graph[i][j] = 0;
            else if (abs(i - j) == 1) graph[i][j] = 1; // Conexión a los nodos adyacentes
            else graph[i][j] = INF;
        }
    }

    floydWarshall(graph);
    return 0;
}
