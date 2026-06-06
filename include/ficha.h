#ifndef FICHA_H
#define FICHA_H

#include <pthread.h>
#include "tablero.h"

typedef struct {
    int jugador;
    int indiceFicha;
    EstadoTablero* tablero;
    int identificadorCola;

    // El proceso jugador despierta al hilo elegido
    pthread_mutex_t* mutexTurnoFicha;
    pthread_cond_t* condTurnoFicha;
    int* fichaTurnoActual; // apunta al elegido por elegirMejorFicha
    int* valorDadoActual;
    bool* juegoActivo;
} ContextoHiloFicha;

void* ejecutarCicloFicha(void* argContexto);

#endif // FICHA_H
