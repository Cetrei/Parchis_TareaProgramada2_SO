#ifndef RENDER_H
#define RENDER_H

#include "tablero.h"

typedef struct {
    int fila;
    int columna;
} CoordenadaTablero;

void limpiarConsola(void);
void dibujarTablero(const EstadoTablero* tablero);
void dibujarEstadisticas(const EstadoTablero* tablero);
void renderizarFrame(const EstadoTablero* tablero);

#endif // RENDER_H
