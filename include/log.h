#ifndef LOG_H
#define LOG_H

#include "config.h"

void abrirArchivoLog(void);
void cerrarArchivoLog(void);
void logDebug(const char* formato, ...);
void registrarMovimiento(int jugador, int indiceFicha, int valorDado, const char* descripcion);
void registrarEvento(int jugador, int indiceFicha, const char* evento);

// Nombres legibles de jugadores para el log
extern const char* NOMBRE_JUGADOR[TOTAL_JUGADORES];

#endif // LOG_H
