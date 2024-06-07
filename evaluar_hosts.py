import subprocess
import os

def is_connectable(host):
    """
    Función que verifica si se puede hacer ping a un host.
    Retorna True si el ping fue exitoso, False en caso contrario.
    """
    try:
        print(f"Checking {host}")
        # Check if SSH port (22) is open
        result = subprocess.run(["nc", "-zv", host, "22"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode != 0:
            return False
        # Try SSH connection with a timeout of 2 seconds and in BatchMode
        result = subprocess.run(["ssh", "-o", "ConnectTimeout=2", "-o", "BatchMode=yes", host, "exit"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(f"Return code: {result.returncode}")
        return result.returncode == 0
    except subprocess.CalledProcessError:
        return False

def main():
    # Lista de strings con números entre 16 and 145
    strings = []
    inicio = 'pcunix'

    for i in range(16, 145):
       string = inicio + str(i) 
       strings.append(string)

    # Abre el archivo en modo de escritura, lo que trunca el archivo a 0 bytes si ya existe
    with open("hosts.txt", "w") as file:
        # Verifica cada host y lo escribe en el archivo si es conectable
        for host in strings:
            if is_connectable(host):
                print(host)
                file.write(host + "\n")
                file.flush()  # Force write to disk

if __name__ == "__main__":
    main()