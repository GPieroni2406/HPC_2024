#include "floyd_paralelo.h"
#define CHUNK_SIZE 2
//#define numHilos 4

// Función para leer la matriz de adyacencia de un grafo desde un archivo.
int **leerGrafoDesdeArchivo(const char *nombreArchivo, int *cantidadVertices) {
    // Intenta abrir el archivo de entrada.
    FILE *archivoEntrada = fopen(nombreArchivo, "r");
    if (archivoEntrada == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    // Lee la cantidad de vértices y asigna espacio para el grafo.
    fscanf(archivoEntrada, "%d", cantidadVertices);
    int **grafo = (int **)malloc(*cantidadVertices * sizeof(int *));
    for (int i = 0; i < *cantidadVertices; i++) {
        grafo[i] = (int *)malloc(*cantidadVertices * sizeof(int));
        for (int j = 0; j < *cantidadVertices; j++) {
            // Lee la matriz de adyacencia del grafo desde el archivo.
            fscanf(archivoEntrada, "%d", &grafo[i][j]);
        }
    }
    fclose(archivoEntrada);
    return grafo;
}

// Función que implementa el algoritmo de Floyd-Warshall paralelizado con OpenMP.
void algoritmoFloydWarshallParalelo(int **grafo, int cantidadVertices,int numHilos) {
    // Asigna memoria para la matriz de distancias.
    int **matrizDistancias = (int **)malloc(cantidadVertices * sizeof(int *));
    for (int i = 0; i < cantidadVertices; i++) {
        matrizDistancias[i] = (int *)malloc(cantidadVertices * sizeof(int));
        // Inicializa la matriz de distancias con los valores del grafo.
        for (int j = 0; j < cantidadVertices; j++) {
            matrizDistancias[i][j] = grafo[i][j];
        }
    }

    // Paraleliza el algoritmo de Floyd-Warshall utilizando OpenMP.
    for (int k = 0; k < cantidadVertices; k++) {
    // La directiva collapse(2) combina los dos bucles for anidados (el bucle 'i' y el bucle 'j')
    // en una sola región de iteraciones. Esto aumenta la granularidad del paralelismo y puede
    // mejorar la distribución de carga de trabajo, especialmente si el bucle más externo (bucle 'k')
    // no tiene suficientes iteraciones para mantener ocupados a todos los hilos.
    // La cláusula schedule(static, cantidadVertices/numHilos) divide las iteraciones de los bucles
    // combinados en bloques estáticos de tamaño 'cantidadVertices/numHilos' y asigna cada bloque a un hilo diferente.
    #pragma omp parallel for schedule(static, cantidadVertices/numHilos) collapse(2)
    for (int i = 0; i < cantidadVertices; i++) {
        for (int j = 0; j < cantidadVertices; j++) {
            // Sección crítica para evitar condiciones de carrera cuando varios hilos intentan
            // actualizar la misma ubicación de memoria de 'matrizDistancias'. La sección crítica
            // asegura que solo un hilo a la vez pueda ejecutar el bloque de código que actualiza
            // la matriz, aunque esto puede introducir un cuello de botella y afectar el rendimiento.
            #pragma omp critical
            {
                // Actualiza la matriz de distancias si se encuentra un camino más corto.
                if (matrizDistancias[i][k] + matrizDistancias[k][j] < matrizDistancias[i][j]) {
                    matrizDistancias[i][j] = matrizDistancias[i][k] + matrizDistancias[k][j];
                }
            }
        }
    }
}
    #pragma omp single
    // Imprime la matriz de distancias más cortas.
    mostrarMatrizDeDistancias(matrizDistancias, cantidadVertices,numHilos);

    // Libera la memoria asignada a la matriz de distancias.
    for (int i = 0; i < cantidadVertices; i++) {
        free(matrizDistancias[i]);
    }
    free(matrizDistancias);
}

// Función para imprimir la matriz de distancias más cortas entre cada par de vértices.
void mostrarMatrizDeDistancias(int **matrizDistancias, int cantidadVertices,int numHilos) {
    printf("La siguiente matriz muestra las distancias más cortas entre cada par de vértices\n");
    // Paraleliza la impresión de la matriz de distancias.
    #pragma omp parallel for schedule(static, cantidadVertices/numHilos)
    for (int i = 0; i < cantidadVertices; i++) {
        for (int j = 0; j < cantidadVertices; j++) {
            // Imprime cada distancia, tratando de manera especial las distancias infinitas.
            if (matrizDistancias[i][j] == DIS_INFINITA)
                printf("%s   ", "DIS_INFINITA");
            else
                printf("%d     ", matrizDistancias[i][j]);
        }
        printf("\n");
    }
}

// La función principal del programa.
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <numero_de_hilos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parsea el número de hilos de los argumentos de la línea de comandos.
    int numHilos = atoi(argv[1]);
    if (numHilos <= 0) {
        fprintf(stderr, "El número de hilos debe ser un entero positivo.\n");
        exit(EXIT_FAILURE);
    }
    int cantidadVertices;
    // Lee el grafo desde el archivo.
    int **grafo = leerGrafoDesdeArchivo("input.txt", &cantidadVertices);
    if (grafo == NULL) {
        return 1; // Termina si no se pudo leer el grafo.
    }

    // Variables para medir el tiempo de ejecución.
    struct timeval tiempoInicio;
    struct timezone zonaHorariaInicio;
    struct timeval tiempoFinal;
    struct timezone zonaHorariaFinal;
    long tiempoInicioMicroseg, tiempoFinalMicroseg;
    double tiempoTotal;

    // Establece el número de hilos a utilizar por OpenMP.
    omp_set_num_threads(numHilos);

    // Comienza la medición del tiempo.
    gettimeofday(&tiempoInicio, &zonaHorariaInicio);

    // Crea una región paralela y ejecuta el algoritmo de Floyd-Warshall en un solo hilo (esto podría revisarse ya que la región paralela no es necesaria aquí).
    #pragma omp parallel default(shared)
        algoritmoFloydWarshallParalelo(grafo, cantidadVertices,numHilos);

    // Termina la medición del tiempo.
    gettimeofday(&tiempoFinal, &zonaHorariaFinal);

    // Calcula y muestra el tiempo total de ejecución.
    tiempoInicioMicroseg = tiempoInicio.tv_sec * 1000000 + tiempoInicio.tv_usec;
    tiempoFinalMicroseg = tiempoFinal.tv_sec * 1000000 + tiempoFinal.tv_usec;
    tiempoTotal = (tiempoFinalMicroseg - tiempoInicioMicroseg) / 1000000.0;
    printf("\nTiempo en segundos (t): %lf\n", tiempoTotal);

    // Libera la memoria asignada al grafo.
    for (int i = 0; i < cantidadVertices; i++) {
        free(grafo[i]);
    }
    free(grafo);

    return 0;
}