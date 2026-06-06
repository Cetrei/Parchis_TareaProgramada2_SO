#include "ficha.h"
#include "reglas.h"
#include "ia.h"
#include "ipc.h"
#include <stdio.h>

// Espera a que el proceso jugador designe a esta ficha como la elegida
static void esperarSerElegida(ContextoHiloFicha* contexto) {
    pthread_mutex_lock(contexto->mutexTurnoFicha);

    while (*contexto->fichaTurnoActual != contexto->indiceFicha &&* contexto->juegoActivo) {
        pthread_cond_wait(contexto->condTurnoFicha, contexto->mutexTurnoFicha);
    }
    pthread_mutex_unlock(contexto->mutexTurnoFicha);
}

static void notificarMovimientoACola(ContextoHiloFicha* contexto, int tipoEvento) {
    MensajeCola evento = {
        .tipoMensaje = tipoEvento,
        .jugadorEmisor = contexto->jugador,
        .jugadorAfectado = contexto->jugador,
        .indiceFicha = contexto->indiceFicha
    };
    enviarEventoCola(contexto->identificadorCola, &evento);
}

static void ejecutarMovimiento(ContextoHiloFicha* contexto) {
    EstadoTablero* tablero = contexto->tablero;
    int jugador = contexto->jugador;
    int indiceFicha = contexto->indiceFicha;
    int valorDado =* contexto->valorDadoActual;

    DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];

    bool eraCapturaPosible = puedeCapturarRival(ficha, valorDado, jugador, tablero);
    aplicarMovimiento(tablero, jugador, indiceFicha, valorDado);

    if (eraCapturaPosible) {
        notificarMovimientoACola(contexto, TIPO_EVENTO_CAPTURA);
    }
    if (ficha->estado == FICHA_EN_META) {
        notificarMovimientoACola(contexto, TIPO_EVENTO_META);
    }
}

static void marcarFichaComoInactiva(ContextoHiloFicha* contexto) {
    pthread_mutex_lock(contexto->mutexTurnoFicha);
    // Senal al proceso jugador de que el movimiento termino
   * contexto->fichaTurnoActual = -1;
    pthread_cond_broadcast(contexto->condTurnoFicha);
    pthread_mutex_unlock(contexto->mutexTurnoFicha);
}

void* ejecutarCicloFicha(void* argContexto) {
    ContextoHiloFicha* contexto = (ContextoHiloFicha* )argContexto;

    while (*contexto->juegoActivo) {
        esperarSerElegida(contexto);

        if (!*contexto->juegoActivo) break;
        
        ejecutarMovimiento(contexto);
        marcarFichaComoInactiva(contexto);
    }
    return NULL;
}
