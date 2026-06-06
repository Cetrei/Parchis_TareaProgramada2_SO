#ifndef IA_H
#define IA_H

#include "tablero.h"

// Retorna el indice de la ficha a mover (0..3), o -1 si ninguna puede moverse
typedef int (*FnEstrategia)(const EstadoTablero* tablero, int jugador, int valorDado);

bool puedeAlcanzarMeta(const DatosFicha* ficha, int valorDado, int jugador);
bool puedeCapturarRival(const DatosFicha* ficha, int valorDado, int jugador, const EstadoTablero* tablero);
bool puedeFormarBarrera(const DatosFicha* ficha, int valorDado, int jugador, const EstadoTablero* tablero);
bool puedeSalirDeBase(const DatosFicha* ficha, int valorDado);
bool puedeAvanzarSeguro(const DatosFicha* ficha, int valorDado, int jugador, const EstadoTablero* tablero);

int elegirMejorFicha(const EstadoTablero* tablero, int jugador, int valorDado);

#endif // IA_H
