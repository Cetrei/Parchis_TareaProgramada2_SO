#ifndef ARBITRO_H
#define ARBITRO_H

#include "tablero.h"

typedef struct {
    EstadoTablero* tablero;
    int identificadorCola;
    int descriptoresPipeLectura[TOTAL_JUGADORES];
    pid_t pidsJugadores[TOTAL_JUGADORES];
} ContextoArbitro;

void ejecutarBucleDeJuego(ContextoArbitro* contexto);
void recolectarEstadisticasFinales(ContextoArbitro* contexto);
void imprimirResultadosFinales(const EstadoTablero* tablero);

#endif // ARBITRO_H
