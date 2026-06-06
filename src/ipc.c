#include "ipc.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

// Sockets AF_UNIX
int crearSocketArbitro(void) {
    int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == -1) {
        perror("Error con el socket del arbitro");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un direccion;
    memset(&direccion, 0, sizeof(direccion));
    direccion.sun_family = AF_UNIX;
    strncpy(direccion.sun_path, RUTA_SOCKET_ARBITRO, sizeof(direccion.sun_path) - 1);
    unlink(RUTA_SOCKET_ARBITRO);

    if (bind(socketFd, (struct sockaddr* )&direccion, sizeof(direccion)) == -1) {
        perror("Error con el bind del arbitro");
        exit(EXIT_FAILURE);
    }
    if (listen(socketFd, TOTAL_JUGADORES) == -1) {
        perror("Error con el listen del arbitro");
        exit(EXIT_FAILURE);
    }
    return socketFd;
}

int aceptarConexionJugador(int socketServidor) {
    int conexion = accept(socketServidor, NULL, NULL);
    if (conexion == -1) {
        perror("Error con el accept del jugador");
    }
    return conexion;
}

int conectarseAlArbitro(void) {
    int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == -1) {
        perror("Error con el socket del jugador");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un direccion;
    memset(&direccion, 0, sizeof(direccion));
    direccion.sun_family = AF_UNIX;
    strncpy(direccion.sun_path, RUTA_SOCKET_ARBITRO, sizeof(direccion.sun_path) - 1);

    if (connect(socketFd, (struct sockaddr* )&direccion, sizeof(direccion)) == -1) {
        perror("Error con el connect del jugador");
        exit(EXIT_FAILURE);
    }
    return socketFd;
}

void enviarMensajeSocket(int socketFd, const MensajeSocket* mensaje) {
    if (write(socketFd, mensaje, sizeof(MensajeSocket)) == -1) {
        perror("Error con el write del socket");
    }
}

void recibirMensajeSocket(int socketFd, MensajeSocket* mensaje) {
    ssize_t bytesLeidos = read(socketFd, mensaje, sizeof(MensajeSocket));
    if (bytesLeidos == 0) {
        // El otro extremo cerro el socket; forzar fin de juego para salir del loop
        mensaje->tipo = MSG_JUEGO_TERMINADO;
        mensaje->jugadorActivo = -1;
    } else if (bytesLeidos == -1) {
        perror("Error con el read del socket");
        mensaje->tipo = MSG_JUEGO_TERMINADO;
        mensaje->jugadorActivo = -1;
    }
}

void cerrarSocket(int socketFd) {
    close(socketFd);
}

void eliminarSocketArbitro(void) {
    unlink(RUTA_SOCKET_ARBITRO);
}

// Colas de mensajes

int crearColaMensajes(void) {
    int identificadorCola = msgget(CLAVE_COLA_MENSAJES, IPC_CREAT | IPC_EXCL | 0666);
    if (identificadorCola == -1) {
        perror("Error al crear la colar");
        exit(EXIT_FAILURE);
    }
    return identificadorCola;
}

void obtenerColaMensajes(int* identificadorCola) {
   * identificadorCola = msgget(CLAVE_COLA_MENSAJES, 0666);
    if (*identificadorCola == -1) {
        perror("Error obteniendo la cola");
        exit(EXIT_FAILURE);
    }
}

void enviarEventoCola(int identificadorCola, const MensajeCola* mensaje) {
    if (msgsnd(identificadorCola, mensaje, sizeof(MensajeCola) - sizeof(long), 0) == -1) {
        perror("Error enviando evento en la cola");
    }
}

void recibirEventoCola(int identificadorCola, MensajeCola* mensaje, long tipo) {
    msgrcv(identificadorCola, mensaje, sizeof(MensajeCola) - sizeof(long), tipo, IPC_NOWAIT);
}

void eliminarColaMensajes(int identificadorCola) {
    msgctl(identificadorCola, IPC_RMID, NULL);
}

// Pipes

void enviarEstadisticasAlPadre(int descriptorEscritura, const EstadisticasFinales* stats) {
    write(descriptorEscritura, stats, sizeof(EstadisticasFinales));
}

void leerEstadisticasDeHijo(int descriptorLectura, EstadisticasFinales* stats) {
    read(descriptorLectura, stats, sizeof(EstadisticasFinales));
}
