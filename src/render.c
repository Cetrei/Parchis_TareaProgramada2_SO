#include "render.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


static const char* TABLERO_BASE[FILAS_TABLERO] = {
     "+----------------------------------+--------------+----------------------------------+",
     "| JUGADOR ROJO                     | 35   34   33 | JUGADOR AZUL                     |",
     "|                                  | 36 +----+ 32 |                                  |",
     "|                                  | 37 | 01 | 30 |                                  |",
     "|                                  | 38 | 02 | 29 |                                  |",
     "|                                  | 39 | 03 | 28 |                                  |",
     "|                                  | 40 | 04 | 27 |                                  |",
     "|                                  | 41 | 05 | 26 |                                  |",
     "+--------------------|  |----------+ 42 | 06 | 25 +----------|   |-------------------+",
     "| 50   49   48   47   46   45   44   43 | 07 | 25   24   23   22   21   20   19   18 |",
    "|    +----------------------------------+----+----------------------------------+    |",
    "| 51 | 01   02   03   04   05   06   07 |    | 07   06   05   04   03   02   01 | 17 |",
    "|    +----------------------------------+----+----------------------------------+    |",
    "| 52   53   54   55   56   57   58   59 | 07 | 09   10   11   12   13   14   15   16 |",
    "+--------------------|  |----------+ 60 | 06 | 08 +----------|  |--------------------+",
    "| JUGADOR VERDE                    | 61 | 05 | 07 | JUGADOR AMARILLO                 |",
    "|                                  | 62 | 04 | 06 |                                  |",
    "|                                  | 63 | 03 | 05 |                                  |",
    "|                                  | 64 | 02 | 04 |                                  |",
    "|                                  | 65 | 01 | 03 |                                  |",
    "|                                  | 66 +----+ 02 |                                  |",
    "|                                  | 67   68   01 |                                  |",
    "+----------------------------------+--------------+----------------------------------+"
};

typedef struct { int fila; int columna; } PosicionEnPantalla;

typedef enum {
    FONDO_NINGUNO = 0,
    FONDO_ROJO,
    FONDO_VERDE,
    FONDO_AMARILLO,
    FONDO_AZUL
} ColorFondoRegion;

// auxiliares
static bool enRango(int valor, int minimo, int maximo) {
    return valor >= minimo && valor <= maximo;
}

static bool enRectangulo(int fila, int columna, int filaInicio, int columnaInicio, int filaFin, int columnaFin) {
    return enRango(fila, filaInicio, filaFin) && enRango(columna, columnaInicio, columnaFin);
}

static int minimo(int primero, int segundo) {
    return primero < segundo ? primero : segundo;
}

static int maximo(int primero, int segundo) {
    return primero > segundo ? primero : segundo;
}

#define CASILLAS_PASILLO (CASILLA_META - INICIO_PASILLO)

/* Array de mapeo:
 *   0                  : no usado
 *   1:CASILLAS_ANILLO  : anillo
 *   69:75              : pasillo rojo
 *   76:82              : pasillo verde
 *   83:89              : pasillo amarillo
 *   90:96              : pasillo azul
 * Total: 97 posiciones (0 a 96).
 * 
 * Los pasillos se organizan como bloques contiguos en memoria (segmentos aplanados).
 * Esto imita una memoria segmentada donde cada jugador tiene su propio bloque:
 *   INICIO_PASILLO (69) + jugador * CASILLAS_PASILLO + paso
 * 
 */
static const PosicionEnPantalla MAPA_COMPLETO[] = {
    {0 , 0 }, // 0 - no usado

    // Anillo 1:68
    {21, 47},
    {20, 47},
    {19, 47},
    {18, 47},
    {17, 47},
    {16, 47},
    {15, 47},
    {14, 47},
    {13, 47},
    {13, 52},
    {13, 57},
    {13, 62},
    {13, 67},
    {13, 72},
    {13, 77},
    {13, 82},
    {11, 82},
    {9 , 82},
    {9 , 77},
    {9, 72 },
    {9, 67 },
    {9, 62 },
    {9 , 57},
    {9 , 52},
    {9, 47},
    {8 , 47},
    {7 , 47},
    {6, 47 },
    {5, 47 },
    {4, 47 },
    {3 , 47},
    {2 , 47},
    {1, 47 },
    {1, 42 },
    {1 , 37},
    {2 , 37},
    {3, 37 },
    {4, 37 },
    {5, 37 },
    {6 , 37},
    {7 , 37},
    {8, 37 },
    {9, 37 },
    {9, 32 },
    {9 , 27},
    {9, 22 },
    {9, 17 },
    {9, 12 },
    {9 , 7 },
    {9 , 2 },
    {11, 2 },
    {13, 2 },
    {13, 7 },
    {13, 12},
    {13, 17},
    {13, 22},
    {13, 27},
    {13, 32},
    {13, 37},
    {14, 37},
    {15, 37},
    {16, 37},
    {17, 37},
    {18, 37},
    {19, 37},
    {20, 37},
    {21, 37},
    {21, 42},

    // Pasillo Rojo 69:75
    {3, 42},
    {4, 42},
    {5, 42},
    {6, 42},
    {7, 42},
    {8, 42},
    {9, 42},

    // Pasillo Verde 76:82
    {11, 7},
    {11, 12},
    {11, 17},
    {11, 22},
    {11, 27},
    {11, 32},
    {11, 37},

    // Pasillo Amarillo 83:89
    {19, 42},
    {18, 42},
    {17, 42},
    {16, 42},
    {15, 42},
    {14, 42},
    {13, 42},

    // Pasillo Azul 90:96
    {11, 77},
    {11, 72},
    {11, 67},
    {11, 62},
    {11, 57},
    {11, 52},
    {11, 47}
};

static PosicionEnPantalla mapearPosicion(int indice) {
    PosicionEnPantalla invalida = {0, 0};
    int totalPosiciones = sizeof(MAPA_COMPLETO) / sizeof(MAPA_COMPLETO[0]);
    if (indice < 0 || indice >= totalPosiciones) {
        return invalida;
    }
    return MAPA_COMPLETO[indice];
}

static void logMapeo(int jugador, int indiceFicha, int posicionLogica, int indiceArray, PosicionEnPantalla pos) {
    static FILE* archivoMapeo = NULL;
    if (!archivoMapeo) archivoMapeo = fopen("mapeo.log", "w");
    if (!archivoMapeo) return;

    static const char INICIALES[TOTAL_JUGADORES] = {'R', 'V', 'A', 'Z'};
    fprintf(archivoMapeo, "[MAPEO] %c%d pos=%d idx=%d -> fila=%d col=%d\n",
        INICIALES[jugador], indiceFicha + 1,
        posicionLogica, indiceArray,
        pos.fila, pos.columna
    );
    fflush(archivoMapeo);
}

/* Convierte estado de ficha a indice del array
 * Si esta en anillo devuelve la posicion directamente (1:68).
 * Si esta en pasillo: el array unificado tiene los pasillos como bloques contiguos.
 */
static int indiceUnificadoDeFicha(int jugador, const DatosFicha* ficha) {
    if (ficha->estado == FICHA_EN_ANILLO) {
        return ficha->posicion;
    }

    int inicioArrayPasillos = INICIO_PASILLO;
    int casillasPorJugadorPasillo = CASILLAS_PASILLO;
    int desplazamientoBloqueJugador = jugador * casillasPorJugadorPasillo;
    int posicionRelativaEnPasillo = ficha->posicion - inicioArrayPasillos;
    int indiceFinal = inicioArrayPasillos + desplazamientoBloqueJugador + posicionRelativaEnPasillo;

    return indiceFinal;
}

/* Cada casilla en el tablero renderizado ocupa 4 columnas de ancho visual:
 *   [espacio][digito1][digito2][espacio]
 * La posicion del mapeo apunta al primer digito (columna central).
 * 
 * Para cubrir toda la casilla visualmente se expande:
 *   -1 a la izquierda (el espacio previo)
 *   +2 a la derecha (el segundo digito + el espacio siguiente)
 * 
 * Esto da un rango de 4 columnas: columna-1, columna, columna+1, columna+2.
 */
static bool coordenadaEnCasillaVisual(PosicionEnPantalla centro, int fila, int columna) {
    return fila == centro.fila && enRango(columna, centro.columna - 1, centro.columna + 2);
}

/* 
 * La expansion visual funciona asi:
 *   - columnaMinima - 1: expande hacia la izquierda desde la primera casilla.
 *   - columnaMaxima + 2: expande hacia la derecha desde la ultima casilla.
 * 
 * En pasillos verticales (rojo, amarillo) la primera y ultima casilla comparten la misma fila
 *  por tanto columnaMinima == columnaMaxima y el ancho es fijo.
 * La altura varia desde la primera hasta la ultima fila.
 * 
 * En pasillos horizontales (verde, azul) la primera y ultima casilla comparten la misma fila
 *  por tanto filaMinima == filaMaxima y la altura es fija.
 * El ancho varia desde la expansion izquierda de la primera casilla hasta la expansion derecha de la ultima casilla.
 */
static bool coordenadaEnRegionPasillo(PosicionEnPantalla extremoA, PosicionEnPantalla extremoB, int fila, int columna) {
    int filaMinima = minimo(extremoA.fila, extremoB.fila);
    int filaMaxima = maximo(extremoA.fila, extremoB.fila);
    int columnaMinima = minimo(extremoA.columna, extremoB.columna);
    int columnaMaxima = maximo(extremoA.columna, extremoB.columna);

    int columnaIzquierda = columnaMinima - 1;
    int columnaDerecha = columnaMaxima + 2;

    return enRectangulo(fila, columna, filaMinima, columnaIzquierda, filaMaxima, columnaDerecha);
}

/* Determina el color de fondo de una coordenada del buffer.
 * Usa el mapeo para obtener las posiciones reales de las salidas y de los extremos de cada pasillo.
 *
 * El orden jugador->ColorFondoRegion es directo: JUGADOR_ROJO=0 -> FONDO_ROJO=1, etc.
 * El indice de fin suma CASILLAS_PASILLO - 1 porque hay 7 casillas (indices 0:6).
 * Por ejemplo, el rojo ocupa 69:75, que son 7 posiciones (75 - 69 + 1 = 7).
 */
static ColorFondoRegion colorFondoEnCoordenada(int fila, int columna) {
    static const int SALIDAS[TOTAL_JUGADORES] = {
        SALIDA_ROJO, SALIDA_VERDE, SALIDA_AMARILLO, SALIDA_AZUL
    };

    // Casillas de salida al anillo (coloreadas por dueno)
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        PosicionEnPantalla posicion = mapearPosicion(SALIDAS[jugador]);
        if (coordenadaEnCasillaVisual(posicion, fila, columna)) return (ColorFondoRegion)(jugador + 1);
    }

    /* Pasillos a meta.
     * Se calculan los indices de inicio y fin de cada bloque usando la formula
     * de memoria segmentada: INICIO_PASILLO + jugador * CASILLAS_PASILLO.
     */
    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        int indiceInicio = INICIO_PASILLO + jugador * CASILLAS_PASILLO;
        int indiceFin = indiceInicio + CASILLAS_PASILLO - 1;
        PosicionEnPantalla inicio = mapearPosicion(indiceInicio);
        PosicionEnPantalla fin    = mapearPosicion(indiceFin);
        if (coordenadaEnRegionPasillo(inicio, fin, fila, columna)) return (ColorFondoRegion)(jugador + 1);
    }

    return FONDO_NINGUNO;
}

static const char* colorFondoPorIndice(ColorFondoRegion indice) {
    static const char* COLORES[5] = {
        NULL,
        COLOR_ROJO_BG,
        COLOR_VERDE_BG,
        COLOR_AMARILLO_BG,
        COLOR_AZUL_BG
    };
    return COLORES[(int)indice];
}

static const char* colorDeJugador(int jugador) {
    static const char* COLORES[TOTAL_JUGADORES] = {
        COLOR_ROJO,
        COLOR_VERDE,
        COLOR_AMARILLO,
        COLOR_AZUL
    };
    return COLORES[jugador];
}

static char inicialDeJugador(int jugador) {
    static const char INICIALES[TOTAL_JUGADORES] = {'R', 'V', 'A', 'Z'};
    return INICIALES[jugador];
}

static void copiarTableroBase(char buffer[FILAS_TABLERO][COLUMNAS_TABLERO + 1]) {
    for (int fila = 0; fila < FILAS_TABLERO; fila++) {
        strncpy(buffer[fila], TABLERO_BASE[fila], COLUMNAS_TABLERO);
        buffer[fila][COLUMNAS_TABLERO] = '\0';
    }
}

static void imprimirBufferConFichas(char buffer[FILAS_TABLERO][COLUMNAS_TABLERO + 1],
    const EstadoTablero* tablero) {

    for (int fila = 0; fila < FILAS_TABLERO; fila++) {
        int columna = 0;
        while (columna < COLUMNAS_TABLERO) {
            ColorFondoRegion colorFondo = colorFondoEnCoordenada(fila, columna);
            if (colorFondo != FONDO_NINGUNO) {
                printf("%s", colorFondoPorIndice(colorFondo));
                while (columna < COLUMNAS_TABLERO && colorFondoEnCoordenada(fila, columna) == colorFondo) {
                    putchar(buffer[fila][columna]);
                    columna++;
                }
                printf("%s", COLOR_RESET);
            } else {
                putchar(buffer[fila][columna]);
                columna++;
            }
        }
        putchar('\n');
    }

    fflush(stdout);

    printf("\033[%dA", FILAS_TABLERO);

    typedef struct {
        int fila, columna;
        int jugador[2];
        int indiceFicha[2];
        int cantidad;
    } CeldaFicha;

    CeldaFicha celdas[TOTAL_JUGADORES * FICHAS_POR_JUGADOR];
    int cantidadCeldas = 0;

    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
            const DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];
            if (ficha->estado == FICHA_EN_BASE || ficha->estado == FICHA_EN_META) continue;

            int indice = indiceUnificadoDeFicha(jugador, ficha);
            PosicionEnPantalla posicion = mapearPosicion(indice);
            if (DEBUG_MAPEO) logMapeo(jugador, indiceFicha, ficha->posicion, indice, posicion);

            int encontrada = -1;
            for (int celda = 0; celda < cantidadCeldas; celda++) {
                if (celdas[celda].fila == posicion.fila && celdas[celda].columna == posicion.columna) {
                    encontrada = celda;
                    break;
                }
            }
            if (encontrada == -1) {
                celdas[cantidadCeldas].fila = posicion.fila;
                celdas[cantidadCeldas].columna = posicion.columna;
                celdas[cantidadCeldas].cantidad = 0;
                encontrada = cantidadCeldas++;
            }
            if (celdas[encontrada].cantidad < 2) {
                celdas[encontrada].jugador[celdas[encontrada].cantidad] = jugador;
                celdas[encontrada].indiceFicha[celdas[encontrada].cantidad] = indiceFicha;
                celdas[encontrada].cantidad++;
            }
        }
    }

    for (int celda = 0; celda < cantidadCeldas; celda++) {
        int columnaBase = celdas[celda].columna;
        if (celdas[celda].cantidad == 2) {
            columnaBase--;
        }

        printf("\033[%d;%dH", celdas[celda].fila + 1, columnaBase + 1);

        ColorFondoRegion fondo = colorFondoEnCoordenada(celdas[celda].fila, columnaBase);
        const char* colorFondo = colorFondoPorIndice(fondo);
        if (colorFondo) printf("%s", colorFondo);

        if (celdas[celda].cantidad == 1) {
            int jugador = celdas[celda].jugador[0];
            int indiceFicha = celdas[celda].indiceFicha[0];
            printf("%s%c%d%s", colorDeJugador(jugador), inicialDeJugador(jugador), indiceFicha + 1, RESET_TEXTO);
        } else {
            int jugadorPrimero = celdas[celda].jugador[0], indicePrimero = celdas[celda].indiceFicha[0];
            int jugadorSegundo = celdas[celda].jugador[1], indiceSegundo = celdas[celda].indiceFicha[1];
            printf("%s%c%d%s,%s%d%s",
                colorDeJugador(jugadorPrimero),
                inicialDeJugador(jugadorPrimero),
                indicePrimero + 1, RESET_TEXTO,
                colorDeJugador(jugadorSegundo),
                indiceSegundo + 1, RESET_TEXTO
            );
        }

        printf("%s", COLOR_RESET);
    }

    fflush(stdout);
    printf("\033[%d;1H", FILAS_TABLERO + 1);
}

void limpiarConsola(void) {
    system("clear");
}

void dibujarTablero(const EstadoTablero* tablero) {
    char buffer[FILAS_TABLERO][COLUMNAS_TABLERO + 1];
    copiarTableroBase(buffer);
    imprimirBufferConFichas(buffer, tablero);
}

void dibujarEstadisticas(const EstadoTablero* tablero) {
    if (tablero->juegoTerminado) return;
    static const char* NOMBRES[TOTAL_JUGADORES] = {
        "Rojo",
        "Verde",
        "Amarillo",
        "Azul"
    };

    printf("\n");
    printf("  %-10s  %6s  %11s  %11s\n", "Jugador", "Puntos", "F.Restantes", "Ultimo Tiro");
    printf("  %-10s  %6s  %11s  %11s\n", "-------", "------", "-----------", "-----------");

    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        const EstadisticasJugador* estadisticas = &tablero->estadisticas[jugador];
        int fichasRestantes = FICHAS_POR_JUGADOR - estadisticas->fichasEnMeta;
        printf("  %s%-10s%s  %6d  %11d  %11d\n",
            colorDeJugador(jugador),
            NOMBRES[jugador],
            COLOR_RESET,
            estadisticas->puntos,
            fichasRestantes,
            estadisticas->ultimoTiro
        );
    }
    printf("\n");
}

static void dibujarBannerGanador(const EstadoTablero* tablero) {
    if (!tablero->juegoTerminado) return;

    static const char* NOMBRES[TOTAL_JUGADORES] = {
        "ROJO",
        "VERDE",
        "AMARILLO",
        "AZUL"
    };
    static const char* COLORES[TOTAL_JUGADORES] = {
        COLOR_ROJO, 
        COLOR_VERDE,
        COLOR_AMARILLO,
        COLOR_AZUL
    };

    printf("\n%s=======================================\n", COLOR_NEGRITA);
    printf("        RESULTADO FINAL DEL TORNEO      \n");
    printf("=======================================%s\n\n", COLOR_RESET);

    if (tablero->juegoTerminado) {
        int ganador = tablero->jugadorGanador;
        printf("  %s%s*** TROFEO -> JUGADOR %s ***%s\n\n",
            COLOR_NEGRITA, COLORES[ganador],
            NOMBRES[ganador],
            COLOR_RESET
        );
    }

    printf("  %-10s  %6s  %7s  %14s\n", "Jugador", "Puntos", "En Meta", "Fichas Comidas");
    printf("  %-10s  %6s  %7s  %14s\n", "-------", "------", "-------", "--------------");

    for (int jugador = 0; jugador < TOTAL_JUGADORES; jugador++) {
        const EstadisticasJugador* stats = &tablero->estadisticas[jugador];
        printf("  %s%-10s%s  %6d  %5d/4  %14d\n",
            COLORES[jugador],
            NOMBRES[jugador],
            COLOR_RESET,
            stats->puntos,
            stats->fichasEnMeta,
            stats->fichasComidas
        );
    }
    printf("\n");
}

void renderizarFrame(const EstadoTablero* tablero) {
    limpiarConsola();
    dibujarTablero(tablero);
    dibujarBannerGanador(tablero);
    dibujarEstadisticas(tablero);

    struct timespec espera = {
        .tv_sec = configuracionGlobal.delayMilisegundos / 1000,
        .tv_nsec = (configuracionGlobal.delayMilisegundos % 1000) * 1000000L
    };
    nanosleep(&espera, NULL);
}