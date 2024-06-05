#ifndef FLOYD_DISTRIBUIDO_H
#define FLOYD_DISTRIBUIDO_H

#include <stdio.h>
#include <stdlib.h>

#define ERROR_MESSAGE // if this constant is defined, show_distances() will print an error message
//#undef ERROR_MESSAGE // undefining this constant, show_distances() won't print anything
#define WRITE_DISTANCES_TO_FILE // define this constant only when you want to write the output to a file (useful when NUMBER_OF_NODES is > 10)
#undef WRITE_DISTANCES_TO_FILE

// Códigos de error personalizados para abortar con mensajes de error.
#define NEGATIVE_NODE_NUMBER -1
#define OUT_OF_MEMORY -2
#define STEP_LESS_THAN_ONE -3

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

// Prototipos de funciones para la creación, llenado y liberación de la matriz de adyacencia.
int **crear_matriz_contigua(int num_nodos);
int **llenar_matriz(int *num_nodos);
void mostrar_matriz(int **distancias, int num_nodos);
void liberar_recursos(int **distancias, int num_nodos);
void abortar_con_error(int codigo_error);
#endif
