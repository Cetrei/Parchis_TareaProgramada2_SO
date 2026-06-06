#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

// Mensajes por socket (arbitro -> jugador)
typedef enum {
    MSG_TURNO_ASIGNADO = 1,
    MSG_JUEGO_TERMINADO = 2,
    MSG_TURNO_COMPLETADO = 3
} TipoMensajeSocket;

typedef struct {
    TipoMensajeSocket tipo;
    int jugadorActivo;
} MensajeSocket;

// Mensajes por cola (jugador -> jugadores: eventos del juego)
#define TIPO_EVENTO_CAPTURA  10L
#define TIPO_EVENTO_META     20L

typedef struct {
    long tipoMensaje;
    int jugadorEmisor;
    int jugadorAfectado;
    int indiceFicha;
} MensajeCola;

// Estadisticas enviadas por pipe (hijo -> padre)
typedef struct {
    int jugador;
    int fichasEnMeta;
    int fichasComidas;
    int puntos;
} EstadisticasFinales;

// API de sockets
int crearSocketArbitro(void);
int aceptarConexionJugador(int socketServidor);
int conectarseAlArbitro(void);
void enviarMensajeSocket(int socketFd, const MensajeSocket* mensaje);
void recibirMensajeSocket(int socketFd, MensajeSocket* mensaje);
void cerrarSocket(int socketFd);
void eliminarSocketArbitro(void);

// API de colas de mensajes
int crearColaMensajes(void);
void obtenerColaMensajes(int* identificadorCola);
void enviarEventoCola(int identificadorCola, const MensajeCola* mensaje);
void recibirEventoCola(int identificadorCola, MensajeCola* mensaje, long tipo);
void eliminarColaMensajes(int identificadorCola);

// API de pipes
void enviarEstadisticasAlPadre(int descriptorEscritura, const EstadisticasFinales* stats);
void leerEstadisticasDeHijo(int descriptorLectura, EstadisticasFinales* stats);

#endif // IPC_H
