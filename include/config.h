#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Ajustes de ejecucion
#define ACTIVAR_LOGS      true
#define PRINT_INFO        false
#define DEBUG_MAPEO       true
#define DELAY_TURNOS_MS   50 

// Constantes de reglas y puntuacion
#define PUNTOS_POR_COMER 0
#define PUNTOS_POR_LLEGAR_META 10


/* NOTA SUPER IMPORTANTE
 * Esta parte usa macros paara evitar hardcodear
 * Pero a diferencia de los ajustes antes de esta nota, los de abajo tienen una logica y cambiarlos podria
 * hacer que el codigo no funcione o tenga bugs
 * (excluyendo los colores)
*/


// Constantes del Tablero
#define TOTAL_CASILLAS       76
#define CASILLAS_ANILLO      68
#define INICIO_PASILLO       69
#define CASILLA_META         76
#define FICHAS_POR_JUGADOR   4
#define TOTAL_JUGADORES      4

// Salidas al anillo de cada color (casillas de inicio)
#define SALIDA_ROJO          46
#define SALIDA_VERDE         56
#define SALIDA_AMARILLO      12
#define SALIDA_AZUL          22

// Entradas al sendero final de cada color
#define ENTRADA_PASILLO_ROJO      34
#define ENTRADA_PASILLO_VERDE     51
#define ENTRADA_PASILLO_AMARILLO  68
#define ENTRADA_PASILLO_AZUL      17

// Constantes de renderizado
#define FILAS_TABLERO    23
#define COLUMNAS_TABLERO 86

// Constantes IPC
#define CLAVE_MEMORIA_COMPARTIDA 0x50415243
#define CLAVE_COLA_MENSAJES      0x50415244
#define RUTA_SOCKET_ARBITRO      "/tmp/parchis_arbitro.sock"
#define NOMBRE_LOG               "debug_parchis.log"

// Colores ANSI
#define COLOR_RESET     "\033[0m"
#define COLOR_NEGRITA   "\033[1m"

#define COLOR_ROJO       "\033[31m"
#define COLOR_VERDE      "\033[32m"
#define COLOR_AMARILLO   "\033[33m"
#define COLOR_AZUL       "\033[34m"

#define COLOR_ROJO_BG      "\033[48;5;52m"
#define COLOR_VERDE_BG     "\033[48;5;22m"
#define COLOR_AMARILLO_BG  "\033[48;5;58m"
#define COLOR_AZUL_BG      "\033[48;5;17m"
#define RESET_TEXTO        "\033[39m"

// Identificadores de jugador
typedef enum {
    JUGADOR_ROJO     = 0,
    JUGADOR_VERDE    = 1,
    JUGADOR_AMARILLO = 2,
    JUGADOR_AZUL     = 3
} IdentificadorJugador;


typedef struct {
    bool activarLogs;
    bool printInfo;
    int delayMilisegundos;
} ConfiguracionSimulador;
extern ConfiguracionSimulador configuracionGlobal; 

void inicializarConfiguracion(bool activarLogs, bool printInfo, int delayMs);

#endif // CONFIG_H
