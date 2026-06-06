#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "config.h"
#include "tablero.h"
#include "arbitro.h"
#include "jugador.h"
#include "ipc.h"
#include "log.h"
#include "ia.h"
#include "render.h"

typedef struct {
    int descriptorLectura;
    int descriptorEscritura;
} ParPipe;

static ParPipe crearPipe(void) {
    ParPipe par;
    int descriptores[2];
    if (pipe(descriptores) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    par.descriptorLectura  = descriptores[0];
    par.descriptorEscritura = descriptores[1];

    return par;
}

static void inicializarEstadoTablero(EstadoTablero* tablero) {
    inicializarMutexesDeCasillas(tablero);
    inicializarSemaforosDeMeta(tablero);
    tablero->juegoTerminado = false;
    tablero->jugadorGanador = -1;

    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        inicializarFichasDeJugador(tablero, jugador);
    }
}

//  Fork + ejecucion del proceso hijo
static pid_t crearProcesoJugador(int jugador, int identificadorShm,
    int identificadorCola, int descriptorEscritura) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error creando proceso hijo");
        exit(EXIT_FAILURE);
    }
    if (pid != 0) return pid;

    HandleMemoriaCompartida handle = conectarseAMemoriaCompartida(identificadorShm);

    // Dar tiempo al arbitro para montar el socket antes de conectar
    usleep(100000);

    ContextoJugador contexto = {
        .jugador = jugador,
        .tablero = handle.tablero,
        .identificadorShm = identificadorShm,
        .identificadorCola = identificadorCola,
        .descriptorPipeEscritura = descriptorEscritura,
        .estrategia = elegirMejorFicha
    };
    ejecutarProcesoJugador(&contexto);
    liberarMemoriaCompartida(&handle, false);
    exit(EXIT_SUCCESS);
}

int main(void) {
    inicializarConfiguracion(ACTIVAR_LOGS, PRINT_INFO, DELAY_TURNOS_MS);
    abrirArchivoLog();

    // Limpiar recursos IPC residuales de ejecuciones anteriores
    shmctl(shmget(CLAVE_MEMORIA_COMPARTIDA, 0, 0666), IPC_RMID, NULL);
    msgctl(msgget(CLAVE_COLA_MENSAJES, 0666), IPC_RMID, NULL);

    HandleMemoriaCompartida handle = crearMemoriaCompartida();
    inicializarEstadoTablero(handle.tablero);

    int identificadorCola = crearColaMensajes();

    ParPipe pipes[TOTAL_JUGADORES];
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        pipes[jugador] = crearPipe();
    }

    ContextoArbitro contextArbitro = {
        .tablero = handle.tablero,
        .identificadorCola = identificadorCola
    };

    // Fork de los 4 procesos jugadores
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        // El hijo hereda el descriptor de escritura; el padre cierra su copia
        pid_t pid = crearProcesoJugador(
            jugador,
            handle.identificadorShm,
            identificadorCola,
            pipes[jugador].descriptorEscritura
        );
        // Proceso padre: guardar PID y cerrar el extremo de escritura del pipe
        contextArbitro.pidsJugadores[jugador] = pid;
        contextArbitro.descriptoresPipeLectura[jugador] = pipes[jugador].descriptorLectura;
        close(pipes[jugador].descriptorEscritura);
    }

    // El padre actua como arbitro
    ejecutarBucleDeJuego(&contextArbitro);
    recolectarEstadisticasFinales(&contextArbitro);

    renderizarFrame(handle.tablero);

    // Limpieza de recursos IPC
    destruirSincronizacion(handle.tablero);
    eliminarColaMensajes(identificadorCola);
    liberarMemoriaCompartida(&handle, true);
    cerrarArchivoLog();

    return EXIT_SUCCESS;
}
