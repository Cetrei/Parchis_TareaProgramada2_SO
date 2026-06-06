#ifndef TABLERO_H
#define TABLERO_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "config.h"

typedef enum {
    FICHA_EN_BASE,
    FICHA_EN_ANILLO,
    FICHA_EN_PASILLO,
    FICHA_EN_META
} EstadoFicha;

typedef struct {
    EstadoFicha estado;
    int posicion;     // 1-68 anillo, 69-75 pasillo, 76 meta, 0 base
    int indiceFicha;  // 0 a 3
    int duenio;       // IdentificadorJugador
} DatosFicha;

typedef struct {
    int puntos;
    int fichasEnMeta;
    int ultimoTiro;
    int fichasComidas;
    int fichasComidaRecibidas;
} EstadisticasJugador;

typedef struct {
    DatosFicha fichas[TOTAL_JUGADORES][FICHAS_POR_JUGADOR];
    EstadisticasJugador estadisticas[TOTAL_JUGADORES];
    bool juegoTerminado;
    int jugadorGanador;

    // Mutex por casilla del anillo (1 a 68)
    pthread_mutex_t mutexCasilla[CASILLAS_ANILLO + 1];

    // Semaforos para zonas restringidas (uno por jugador)
    sem_t semaforoPasillo[TOTAL_JUGADORES];
    sem_t semaforoMeta[TOTAL_JUGADORES];

    // Mutex global para operaciones atomicas sobre el estado
    pthread_mutex_t mutexEstadoGeneral;
} EstadoTablero;

typedef struct {
    EstadoTablero* tablero;
    int identificadorShm;
} HandleMemoriaCompartida;

HandleMemoriaCompartida crearMemoriaCompartida(void);
HandleMemoriaCompartida conectarseAMemoriaCompartida(int identificadorShm);
void liberarMemoriaCompartida(HandleMemoriaCompartida* handle, bool esCreador);

void inicializarMutexesDeCasillas(EstadoTablero* tablero);
void inicializarSemaforosDeMeta(EstadoTablero* tablero);
void destruirSincronizacion(EstadoTablero* tablero);
void inicializarFichasDeJugador(EstadoTablero* tablero, int jugador);

bool casillaTieneBarrera(const EstadoTablero* tablero, int casilla);
bool casillaEsSegura(int casilla, int jugador);
int contarFichasEnCasilla(const EstadoTablero* tablero, int casilla, int jugador);
int encontrarRivalEnCasilla(const EstadoTablero* tablero, int casilla, int jugadorAtacante);

#endif // TABLERO_H
