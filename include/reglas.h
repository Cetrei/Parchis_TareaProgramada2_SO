#ifndef REGLAS_H
#define REGLAS_H

#include "tablero.h"
#include <stdbool.h>

// Consultas base
bool fichaEstaEnJuego(const DatosFicha* ficha);
bool fichaLlegoAMeta(const DatosFicha* ficha);
bool movimientoEsValido(const DatosFicha* ficha, int valorDado, const EstadoTablero* tablero);
int calcularPosicionDestino(const DatosFicha* ficha, int valorDado, int jugador);

// Movimiento
int avanzarEnAnillo(int posicionActual, int pasos);
bool posicionEntraAlPasillo(int posicion, int pasos, int jugador);
int calcularPosicionEnPasillo(const DatosFicha* ficha, int valorDado, int jugador);

// Acciones por movimiento
void devolverFichaABase(EstadoTablero* tablero, int jugador, int indiceFicha);
void aplicarMovimiento(EstadoTablero* tablero, int jugador, int indiceFicha, int valorDado);
bool intentarCapturarRival(EstadoTablero* tablero, int jugadorAtacante, int casilla);
void sacarFichaDeBase(EstadoTablero* tablero, int jugador, int indiceFicha);

extern const int SALIDA_JUGADOR[TOTAL_JUGADORES]; // Puntos de salida al anillo
extern const int ENTRADA_PASILLO[TOTAL_JUGADORES]; // Puntos de entrada al pasillo

#endif // REGLAS_H
