#include "jugador.h"
#include "ficha.h"
#include "ipc.h"
#include "dado.h"
#include "config.h"
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutexTurnoFicha;
    pthread_cond_t condTurnoFicha;
    int fichaTurnoActual;
    int valorDadoActual;
    bool juegoActivo;
} SincronizacionInterna;

static void inicializarSincronizacion(SincronizacionInterna* sinc) {
    pthread_mutex_init(&sinc->mutexTurnoFicha, NULL);
    pthread_cond_init(&sinc->condTurnoFicha, NULL);
    sinc->fichaTurnoActual = -1;
    sinc->valorDadoActual = 0;
    sinc->juegoActivo = true;
}

static void destruirSincronizacionInterna(SincronizacionInterna* sinc) {
    pthread_mutex_destroy(&sinc->mutexTurnoFicha);
    pthread_cond_destroy(&sinc->condTurnoFicha);
}

static void lanzarHilosDeFichas(ContextoJugador* contexto, SincronizacionInterna* sinc,
    ContextoHiloFicha contextosFichas[FICHAS_POR_JUGADOR], pthread_t hilos[FICHAS_POR_JUGADOR]) {
    for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
        ContextoHiloFicha* ctxFicha = &contextosFichas[indiceFicha];
        ctxFicha->jugador           = contexto->jugador;
        ctxFicha->indiceFicha       = indiceFicha;
        ctxFicha->tablero           = contexto->tablero;
        ctxFicha->identificadorCola = contexto->identificadorCola;
        ctxFicha->mutexTurnoFicha   = &sinc->mutexTurnoFicha;
        ctxFicha->condTurnoFicha    = &sinc->condTurnoFicha;
        ctxFicha->fichaTurnoActual  = &sinc->fichaTurnoActual;
        ctxFicha->valorDadoActual   = &sinc->valorDadoActual;
        ctxFicha->juegoActivo       = &sinc->juegoActivo;
        pthread_create(&hilos[indiceFicha], NULL, ejecutarCicloFicha, ctxFicha);
    }
}

static bool todosEnMeta(const EstadoTablero* tablero, int jugador) {
    return tablero->estadisticas[jugador].fichasEnMeta == FICHAS_POR_JUGADOR;
}

static void procesarTurno(ContextoJugador* contexto, SincronizacionInterna* sinc) {
    int valorDado = lanzarDado();
    int jugador = contexto->jugador;
    EstadoTablero* tablero = contexto->tablero;

    tablero->estadisticas[jugador].ultimoTiro = valorDado;

    int fichaDeterminada = contexto->estrategia(tablero, jugador, valorDado);
    if (fichaDeterminada == -1) return;

    pthread_mutex_lock(&sinc->mutexTurnoFicha);
    sinc->valorDadoActual  = valorDado;
    sinc->fichaTurnoActual = fichaDeterminada;
    pthread_cond_broadcast(&sinc->condTurnoFicha);

    while (sinc->fichaTurnoActual != -1) {
        pthread_cond_wait(&sinc->condTurnoFicha, &sinc->mutexTurnoFicha);
    }
    pthread_mutex_unlock(&sinc->mutexTurnoFicha);
}

static void detenerHilos(SincronizacionInterna* sinc, pthread_t hilos[FICHAS_POR_JUGADOR]) {
    pthread_mutex_lock(&sinc->mutexTurnoFicha);
    sinc->juegoActivo = false;
    sinc->fichaTurnoActual = -2;
    pthread_cond_broadcast(&sinc->condTurnoFicha);
    pthread_mutex_unlock(&sinc->mutexTurnoFicha);

    for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
        pthread_join(hilos[indiceFicha], NULL);
    }
}

static void enviarEstadisticasFinales(ContextoJugador* contexto) {
    EstadisticasFinales stats = {
        .jugador       = contexto->jugador,
        .fichasEnMeta  = contexto->tablero->estadisticas[contexto->jugador].fichasEnMeta,
        .fichasComidas = contexto->tablero->estadisticas[contexto->jugador].fichasComidas,
        .puntos        = contexto->tablero->estadisticas[contexto->jugador].puntos
    };
    enviarEstadisticasAlPadre(contexto->descriptorPipeEscritura, &stats);
}

void ejecutarProcesoJugador(ContextoJugador* contexto) {
    inicializarSemillaAleatoria(contexto->jugador * 667 + getpid());

    SincronizacionInterna sinc;
    inicializarSincronizacion(&sinc);

    pthread_t hilos[FICHAS_POR_JUGADOR];
    ContextoHiloFicha contextosFichas[FICHAS_POR_JUGADOR];
    lanzarHilosDeFichas(contexto, &sinc, contextosFichas, hilos);

    int socketJugador = conectarseAlArbitro();
    MensajeSocket mensajeEntrante;

    while (true) {
        recibirMensajeSocket(socketJugador, &mensajeEntrante);

        if (mensajeEntrante.tipo == MSG_JUEGO_TERMINADO) break;

        if (!todosEnMeta(contexto->tablero, contexto->jugador)) {
            procesarTurno(contexto, &sinc);
        }

        MensajeSocket ack = {
            .tipo = MSG_TURNO_COMPLETADO,
            .jugadorActivo = contexto->jugador
        };
        enviarMensajeSocket(socketJugador, &ack);
    }

    cerrarSocket(socketJugador);
    detenerHilos(&sinc, hilos);
    destruirSincronizacionInterna(&sinc);
    enviarEstadisticasFinales(contexto);
    close(contexto->descriptorPipeEscritura);
}
