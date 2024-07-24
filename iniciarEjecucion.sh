#!/bin/bash

NP=${1:-4} # PROCESOS - POR DEFECTO 4
H=${2:-1} # HILOS - POR DEFECTO 1
M=${matrices/32x32.txt} # Archivo de matrices - POR DEFECTO 16 nodos

# Cargar el modulo MPI
module load mpi/mpich-x86_64

# Ejecutar el script Python
python3 evaluar_hosts.py

# Limpiar y compilar
make clean
make

# Ejecutar el programa con mpirun
mpirun -np $NP --hostfile hosts.txt ./floyd_distribuido $H $M