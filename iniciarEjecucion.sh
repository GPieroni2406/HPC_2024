P=${1:-4} # PROCESOS - POR DEFECTO 4
H=${2:-1} # HILOS - POR DEFECTO 1

# Se ejecuta mpirun con el valor de NP
module load mpi/mpich-x86_64
python3 -m pip install -r requirements.txt
python3 evaluar_hosts.py
make clean
make
mpirun -np $NP --hostfile hosts_conectables.txt ./floyd_distribuido_submatrices $H