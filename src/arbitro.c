#include "arbitro.h"
#include "render.h"
#include "ipc.h"
#include "config.h"
#include "log.h"
#include <sys/wait.h>
#include <unistd.h>

#define MAX_TURNOS_TOTALES 4000


static bool verificarGanador(EstadoTablero* tablero) {
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        if (tablero->estadisticas[jugador].fichasEnMeta == FICHAS_POR_JUGADOR) {
            tablero->juegoTerminado = true;
            tablero->jugadorGanador = jugador;
            return true;
        }
    }
    return false;
}

static void notificarTurno(int socketConexiones[TOTAL_JUGADORES], int jugadorActivo) {
    MensajeSocket mensaje = {
        .tipo = MSG_TURNO_ASIGNADO,
        .jugadorActivo = jugadorActivo
    };
    enviarMensajeSocket(socketConexiones[jugadorActivo], &mensaje);
}

static void esperarAckDeTurno(int socketConexiones[TOTAL_JUGADORES], int jugadorActivo) {
    MensajeSocket ack;
    recibirMensajeSocket(socketConexiones[jugadorActivo], &ack);
}

static void notificarFinDeJuego(int socketConexiones[TOTAL_JUGADORES]) {
    MensajeSocket mensaje = {
        .tipo = MSG_JUEGO_TERMINADO,
        .jugadorActivo = -1
    };
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        enviarMensajeSocket(socketConexiones[jugador], &mensaje);
    }
}

static void aceptarConexionesDeJugadores(int socketServidor, int socketConexiones[TOTAL_JUGADORES]) {
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        socketConexiones[jugador] = aceptarConexionJugador(socketServidor);
    }
}

void ejecutarBucleDeJuego(ContextoArbitro* contexto) {
    logDebug("[ARBITRO] Creando socket del arbitro...\n");
    int socketServidor = crearSocketArbitro();

    logDebug("[ARBITRO] Esperando conexion de los 4 jugadores...\n");
    int socketConexiones[TOTAL_JUGADORES];
    aceptarConexionesDeJugadores(socketServidor, socketConexiones);

    logDebug("[ARBITRO] Todos los jugadores conectados. Iniciando bucle de juego.\n");

    int turnosJugados = 0;
    int jugadorActual = JUGADOR_ROJO;

    while (turnosJugados < MAX_TURNOS_TOTALES) {
        logDebug("[ARBITRO] Turno %d -- jugador %d\n", turnosJugados + 1, jugadorActual);

        notificarTurno(socketConexiones, jugadorActual);
        esperarAckDeTurno(socketConexiones, jugadorActual);

        renderizarFrame(contexto->tablero);

        if (verificarGanador(contexto->tablero)) break;

        jugadorActual = (jugadorActual + 1) % TOTAL_JUGADORES;
        turnosJugados++;
    }

    notificarFinDeJuego(socketConexiones);

    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        cerrarSocket(socketConexiones[jugador]);
    }
    cerrarSocket(socketServidor);
    eliminarSocketArbitro();
}

void recolectarEstadisticasFinales(ContextoArbitro* contexto) {
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        EstadisticasFinales stats;
        leerEstadisticasDeHijo(contexto->descriptoresPipeLectura[jugador], &stats);
        close(contexto->descriptoresPipeLectura[jugador]);
    }

    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        int estado;
        waitpid(contexto->pidsJugadores[jugador], &estado, 0);
    }
}