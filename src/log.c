#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

const char* NOMBRE_JUGADOR[TOTAL_JUGADORES] = {
    "ROJO",
    "VERDE",
    "AMARILLO",
    "AZUL"
};

static FILE* archivoLog = NULL;
static pthread_mutex_t mutexLog = PTHREAD_MUTEX_INITIALIZER;

void abrirArchivoLog(void) {
    if (!configuracionGlobal.activarLogs) return;

    archivoLog = fopen(NOMBRE_LOG, "w");
    if (!archivoLog) {
        perror("Error abriendo los logs");
    }
}

void cerrarArchivoLog(void) {
    if (archivoLog) {
        fclose(archivoLog);
        archivoLog = NULL;
    }
}

void logDebug(const char* formato, ...) {
    if (!configuracionGlobal.printInfo) return;
    va_list argumentos;
    va_start(argumentos, formato);
    vfprintf(stderr, formato, argumentos);
    va_end(argumentos);
}

void registrarMovimiento(int jugador, int indiceFicha, int valorDado, const char* descripcion) {
    if (!configuracionGlobal.activarLogs || !archivoLog) return;

    pthread_mutex_lock(&mutexLog);
    fprintf(archivoLog, "[JUGADOR %s][FICHA %d] %s (dado %d)\n",
        NOMBRE_JUGADOR[jugador],
        indiceFicha + 1,
        descripcion,
        valorDado
    );
    fflush(archivoLog);
    pthread_mutex_unlock(&mutexLog);
}

void registrarEvento(int jugador, int indiceFicha, const char* evento) {
    if (!configuracionGlobal.activarLogs || !archivoLog) return;

    pthread_mutex_lock(&mutexLog);
    fprintf(archivoLog, "[JUGADOR %s][FICHA %d] %s\n",
        NOMBRE_JUGADOR[jugador],
        indiceFicha + 1,
        evento
    );
    fflush(archivoLog);
    pthread_mutex_unlock(&mutexLog);
}
