#ifndef JUGADOR_H
#define JUGADOR_H

#include "tablero.h"
#include "ia.h"

typedef struct {
    int jugador;
    EstadoTablero * tablero;
    int identificadorShm;
    int identificadorCola;
    int descriptorPipeEscritura;
    FnEstrategia estrategia; // Puntero a la funcion de IA (intercambiable)
} ContextoJugador;

void ejecutarProcesoJugador(ContextoJugador* contexto);

#endif // JUGADOR_H
