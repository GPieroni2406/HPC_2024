#include "../include/floyd_secuencial.h"

// Funcion para implementar el algoritmo de Floyd-Warshall. Cargo una matriz de distancias a partir del grafo de entradac que luego se ira actualizando.
void algoritmoFloydWarshall(int **grafo, int cantidadVertices) {
    int **matrizDistancias = (int **)malloc(cantidadVertices * sizeof(int *)); //asignar dinamicamente la cantidad de memoria necesaria basada en V.
    for (int vertice1 = 0; vertice1 < cantidadVertices; vertice1++) {
        matrizDistancias[vertice1] = (int *)malloc(cantidadVertices * sizeof(int));
        for (int vertice2 = 0; vertice2 < cantidadVertices; vertice2++) {
            matrizDistancias[vertice1][vertice2] = grafo[vertice1][vertice2];
        }
    }
    // Algoritmo Clasico de Floyd Warshall - O(V^3)
    for (int verticeIntermedio = 0; verticeIntermedio < cantidadVertices; verticeIntermedio++) {
        for (int origen = 0; origen < cantidadVertices; origen++) {
            for (int destino = 0; destino < cantidadVertices; destino++) {
                if (matrizDistancias[origen][verticeIntermedio] + matrizDistancias[verticeIntermedio][destino] < matrizDistancias[origen][destino]) {
                    matrizDistancias[origen][destino] = matrizDistancias[origen][verticeIntermedio] + matrizDistancias[verticeIntermedio][destino];
                }
            }
        }
    }

    mostrarMatrizDeDistancias(matrizDistancias, cantidadVertices);

    for (int i = 0; i < cantidadVertices; i++) {
        free(matrizDistancias[i]);
    }
    free(matrizDistancias);
}

void mostrarMatrizDeDistancias(int **matrizDistancias, int cantidadVertices) {
    printf("La siguiente matriz muestra las distancias mas cortas entre cada par de vertices\n");
    for (int i = 0; i < cantidadVertices; i++) {
        for (int j = 0; j < cantidadVertices; j++) {
            if (matrizDistancias[i][j] == DIS_INFINITA)
                printf("%s    ", "DIS_INFINITA");
            else
                printf("%d      ", matrizDistancias[i][j]);
        }
        printf("\n");
    }
}

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

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Uso: ./floyd_secuencial <ubicacion de matriz> \n");
        exit(EXIT_FAILURE);
    }
    char *ubicacion_matriz = argv[1];
    
    int cantidadVertices;
    int **grafo = leerGrafoDesdeArchivo(ubicacion_matriz, &cantidadVertices);
    if (grafo == NULL) {
        return 1; // Termina si no se pudo leer el grafo
    }

    struct timeval tiempoInicio;
    struct timezone zonaHorariaInicio;
    struct timeval tiempoFinal;
    struct timezone zonaHorariaFinal;
    long tiempoInicioMicroseg, tiempoFinalMicroseg;
    double tiempoTotal;

    gettimeofday(&tiempoInicio, &zonaHorariaInicio);

    algoritmoFloydWarshall(grafo, cantidadVertices);

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