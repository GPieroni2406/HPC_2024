# Instrucciones para el uso y la ejecución del Programa 

El siguiente documento contiene las instrucciones para la compilación y ejecución del programa, así como archivos generados para compilar automáticamente.

## Evaluar Hosts

El archivo de python evaluar_hosts.py sirve para encontrar nodos disponibles en el cluster de la FING para una conexión futura por medio de SSH.
Para ejecutarlo aplicamos el siguiente codigo:

```bash
python3 evaluar_hosts.py
```
Como precondicion se debe tener python instalado.
Como postcondicion se genera un archivo "hosts.txt" con equipos disponibles para ser usados como nodos en MPI

## Matrices de Prueba

Dentro de la carpeta "matrices" se agregaron archivos con diferentes matrices de prueba.

### Compilación del Programa

Dentro de la carpeta principal se dejan los 3 codigos compilados.
Los siguientes comandos sirven para compilar cada uno de ellos:

```bash
Secuencial: 
gcc -o floyd_secuencial floyd_secuencial.c

Paralelo:
gcc -fopenmp -o floyd_paralelo floyd_paralelo.c

Distribuido:
module load mpi/mpich-x86_64 *Para cargar el modulo*
mpicc -fopenmp -o floyd_distribuido floyd_distribuido.c -lm
```

### Ejecución del Programa

Para realizar una ejecución del programa, se utilizan los siguientes comandos:

```bash
Secuencial: 
./floyd_secuencial ubicacionMatriz

Paralelo:
./floyd_paralelo numHilos ubicacionMatriz

Distribuido:
mpirun -np N ./floyd_distribuido
```

# Iniciar Ejecucion de Forma automatica

Dentro de la carpeta principal, se deja un archivo llamado iniciarEjecucion.sh que fue utilizado en las maquinas de la FING. El mismo se encarga de obtener los Hosts disponibles y ejecutar el programa ditribuido con MPI.

# ACLARACION
1) Los archivos con la matriz 4096x4096 y 8000x8000 utilizadas como base de las pruebas fueron comprimidos para no ocupar tanto espacio. Estos pueden descomprimirse y utilizarse sin problema.
2) Es coveniente realizar una re-compilacion del floyd_distribuido en la maquina donde se ejecute, ya que el mismo utiliza el modulo de mpi y trae problemas con la ubicacion.