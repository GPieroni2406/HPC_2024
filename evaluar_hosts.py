# -*- coding: utf-8 -*-
import subprocess
import os

def add_host_key_to_known_hosts(host):
    try:
        result = subprocess.run(["ssh-keyscan", "-H", host], capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            with open(os.path.expanduser("~/.ssh/known_hosts"), "a") as known_hosts:
                known_hosts.write(result.stdout)
            print(f"La clave publica de {host} ha sido agregada a known_hosts.")
            return True
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired, Exception) as e:
        print(f"No se pudo obtener la clave publica de {host}: {e}")
    return False

def is_connectable(host):
    try:
        print(f"Checking {host}")
        # Check if SSH port (22) is open
        result = subprocess.run(["nc", "-zv", host, "22"], capture_output=True, text=True)
        if result.returncode != 0:
            return False
        # Try SSH connection with a timeout of 2 seconds and in BatchMode
        result = subprocess.run(["ssh", "-o", "ConnectTimeout=2", "-o", "BatchMode=yes", host, "exit"], capture_output=True, text=True)
        print(f"Return code: {result.returncode}")
        return result.returncode == 0
    except subprocess.CalledProcessError:
        return False

def main():
    # Lista de strings con números entre 16 y 145
    strings = []
    inicio = 'pcunix'

    for i in range(16, 145):
        string = inicio + str(i)
        strings.append(string)

    # Verifica cada host y, si es necesario, agrega la clave del host a known_hosts
    for host in strings:
        add_host_key_to_known_hosts(host)

    # Abre el archivo en modo de escritura, lo que trunca el archivo a 0 bytes si ya existe
    with open("hosts.txt", "w") as file:
        # Verifica cada host y lo escribe en el archivo si es conectable
        for host in strings:
            if is_connectable(host):
                print(f"{host} is connectable")
                file.write(host + "\n")
                file.flush()  # Force write to disk

if __name__ == "__main__":
    main()