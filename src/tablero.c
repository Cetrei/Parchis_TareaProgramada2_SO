#include "tablero.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Memoria compartida

HandleMemoriaCompartida crearMemoriaCompartida(void) {
    HandleMemoriaCompartida handle;
    handle.identificadorShm = shmget(
        CLAVE_MEMORIA_COMPARTIDA,
        sizeof(EstadoTablero),
        IPC_CREAT | IPC_EXCL | 0666
    );
    if (handle.identificadorShm == -1) {
        perror("Error creando segmento de memoria compartida");
        exit(EXIT_FAILURE);
    }

    handle.tablero = (EstadoTablero*)shmat(handle.identificadorShm, NULL, 0);
    if (handle.tablero == (void* )-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    memset(handle.tablero, 0, sizeof(EstadoTablero));
    return handle;
}

HandleMemoriaCompartida conectarseAMemoriaCompartida(int identificadorShm) {
    HandleMemoriaCompartida handle;
    handle.identificadorShm = identificadorShm;
    handle.tablero = (EstadoTablero* )shmat(identificadorShm, NULL, 0);
    if (handle.tablero == (void* )-1) {
        perror("shmat hijo");
        exit(EXIT_FAILURE);
    }
    return handle;
}

void liberarMemoriaCompartida(HandleMemoriaCompartida* handle, bool esCreador) {
    shmdt(handle->tablero);
    if (esCreador) {
        shmctl(handle->identificadorShm, IPC_RMID, NULL);
    }
}

// Sincronizacion

void inicializarMutexesDeCasillas(EstadoTablero* tablero) {
    pthread_mutexattr_t atributo;
    pthread_mutexattr_init(&atributo);

    pthread_mutexattr_setpshared(&atributo, PTHREAD_PROCESS_SHARED);

    for (int casilla = 0; casilla <= CASILLAS_ANILLO; casilla++) {
        pthread_mutex_init(&tablero->mutexCasilla[casilla], &atributo);
    }

    pthread_mutex_init(&tablero->mutexEstadoGeneral, &atributo);
    pthread_mutexattr_destroy(&atributo);
}

void inicializarSemaforosDeMeta(EstadoTablero* tablero) {
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        // Pasillo estrecho: maximo 1 ficha a la vez por casilla
        sem_init(&tablero->semaforoPasillo[jugador], 1, FICHAS_POR_JUGADOR);
        // Meta: admite hasta FICHAS_POR_JUGADOR fichas
        sem_init(&tablero->semaforoMeta[jugador], 1, FICHAS_POR_JUGADOR);
    }
}

void destruirSincronizacion(EstadoTablero* tablero) {
    for (int casilla = 0; casilla <= CASILLAS_ANILLO; casilla++) {
        pthread_mutex_destroy(&tablero->mutexCasilla[casilla]);
    }

    pthread_mutex_destroy(&tablero->mutexEstadoGeneral);
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        sem_destroy(&tablero->semaforoPasillo[jugador]);
        sem_destroy(&tablero->semaforoMeta[jugador]);
    }
}

void inicializarFichasDeJugador(EstadoTablero* tablero, int jugador) {
    for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
        DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];
        ficha->estado = FICHA_EN_BASE;
        ficha->posicion = 0;
        ficha->indiceFicha = indiceFicha;
        ficha->duenio = jugador;
    }
}

// Estado del tablero

int contarFichasEnCasilla(const EstadoTablero* tablero, int casilla, int jugador) {
    int contador = 0;
    for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
        const DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];
        bool estaEnAnillo = (ficha->estado == FICHA_EN_ANILLO);
        if (estaEnAnillo && ficha->posicion == casilla) {
            contador++;
        }
    }
    return contador;
}

bool casillaTieneBarrera(const EstadoTablero* tablero, int casilla) {
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        if (contarFichasEnCasilla(tablero, casilla, jugador) >= 2) {
            return true;
        }
    }
    return false;
}

bool casillaEsSegura(int casilla, int jugador) {
    int salidasSeguras[TOTAL_JUGADORES] = {
        SALIDA_ROJO,
        SALIDA_VERDE,
        SALIDA_AMARILLO,
        SALIDA_AZUL
    };
    return casilla == salidasSeguras[jugador];
}

// Retorna el indice del jugador rival que tiene UNA sola ficha comible en esa casilla, o -1 si no hay ninguno
int encontrarRivalEnCasilla(const EstadoTablero* tablero, int casilla, int jugadorAtacante) {
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        if (jugador == jugadorAtacante) continue;

        int fichasEnCasilla = contarFichasEnCasilla(tablero, casilla, jugador);
        bool esRivalSolo = (fichasEnCasilla == 1);
        bool esCasillaSeguraDelRival = casillaEsSegura(casilla, jugador);

        if (esRivalSolo && !esCasillaSeguraDelRival) {
            return jugador;
        }
    }
    return -1;
}
