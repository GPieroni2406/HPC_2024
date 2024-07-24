#ifndef FLOYD_SECUENCIAL_H
#define FLOYD_SECUENCIAL_H

#include<limits.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h> //EN WINDOWS SE UTILIZA #include<time.h>

#define DIS_INFINITA 9999

// Prototipos de funciones
void mostrarMatrizDeDistancias(int **matrizDistancias, int cantidadVertices);
void algoritmoFloydWarshall(int **grafo, int cantidadVertices);
int **leerGrafoDesdeArchivo(const char *nombreArchivo, int *cantidadVertices);


#endif // FLOYD_WARSHALL_H