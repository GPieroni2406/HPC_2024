#ifndef FLOYD_PARALELO_H
#define FLOYD_PARALELO_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>
#include <sys/time.h>

#define DIS_INFINITA 9999


// Prototipos de las funciones
void mostrarMatrizDeDistancias(int **matrizDistancias, int cantidadVertices,int numHilos);
int **leerGrafoDesdeArchivo(const char *nombreArchivo, int *cantidadVertices);
void algoritmoFloydWarshallParalelo(int **grafo, int cantidadVertices, int numHilos);

#endif // FLOYD_WARSHALL_H