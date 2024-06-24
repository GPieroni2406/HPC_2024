#!/bin/bash

# Compilar el programa
gcc -fopenmp -o floyd_paralelomio floyd_paralelomio.c

# Ejecutar el programa variando el número de núcleos
for num_threads in 1 2 4 8 20 30; do
    echo "Ejecutando con $num_threads hilos:"
    export OMP_NUM_THREADS=$num_threads
    ./floyd_paralelomio
    echo ""
done
