#ifndef FLOYD_DISTRIBUIDO_SUBMATRICES_H
#define FLOYD_DISTRIBUIDO_SUBMATRICES_H

#include <stdio.h> // Incluir la biblioteca estándar de entrada y salida para el manejo de archivos

#define INF 9999 // Valor utilizado para representar la ausencia de conexión

// Declaraciones de funciones
int min(int a, int b);
int add(int a, int b);
int** readMatrixFromFile(const char* filename, int* rows);
int** allocMatrix(int rows);
void freeMatrix(int** matrix, int rows);
int readVerticesCount(FILE* file);

#endif // FLOYD_DISTRIBUIDO_H