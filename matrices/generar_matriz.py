import numpy as np
import random

# Definir el tamaño de la matriz
N = 8000

# Definir un valor para representar 'infinito'
INF = 9999

# Crear una matriz de 32x32 inicializada con 'infinito'
adj_matrix = np.full((N, N), INF)

# Establecer la diagonal a 0, ya que la distancia de un nodo a sí mismo es 0
np.fill_diagonal(adj_matrix, 0)

# Añadir conexiones aleatorias con una probabilidad, por ejemplo, 50%
connection_probability = 0.4
for i in range(N):
    for j in range(N):
        if i != j and random.random() < connection_probability:
            adj_matrix[i][j] = random.randint(1, 10)  # Peso aleatorio entre 1 y 10

# Guardar la matriz de adyacencia en un archivo de texto
with open('8000x8000.txt', 'w') as f:
    for row in adj_matrix:
        f.write(' '.join(str(int(x)) for x in row) + '\n')