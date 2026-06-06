#include "ia.h"
#include "reglas.h"
#include <stddef.h>


bool puedeAlcanzarMeta(const DatosFicha* ficha, int valorDado, int jugador) {
    if (!fichaEstaEnJuego(ficha)) return false;
    
    int destino = calcularPosicionDestino(ficha, valorDado, jugador);
    return destino == CASILLA_META;
}

bool puedeCapturarRival(const DatosFicha* ficha, int valorDado, int jugador, const EstadoTablero* tablero) {
    if (ficha->estado != FICHA_EN_ANILLO) return false;
    
    int destino = calcularPosicionDestino(ficha, valorDado, jugador);
    bool destinoEnAnillo = (destino >= 1 && destino <= CASILLAS_ANILLO);
    if (!destinoEnAnillo) return false;
    
    return encontrarRivalEnCasilla(tablero, destino, jugador) != -1;
}

bool puedeFormarBarrera(const DatosFicha* ficha, int valorDado, int jugador, const EstadoTablero* tablero) {
    if (ficha->estado != FICHA_EN_ANILLO) return false;
    
    int destino = calcularPosicionDestino(ficha, valorDado, jugador);
    bool destinoEnAnillo = (destino >= 1 && destino <= CASILLAS_ANILLO);
    if (!destinoEnAnillo) return false;
    
    // Si ya hay una ficha propia en el destino, formaremos barrera
    return contarFichasEnCasilla(tablero, destino, jugador) == 1;
}

bool puedeSalirDeBase(const DatosFicha* ficha, int valorDado) {
    return ficha->estado == FICHA_EN_BASE && valorDado == 6;
}

bool puedeAvanzarSeguro(const DatosFicha* ficha, int valorDado, int jugador, const EstadoTablero* tablero) {
    if (!fichaEstaEnJuego(ficha)) return false;
    if (!movimientoEsValido(ficha, valorDado, tablero)) return false;

    int destino = calcularPosicionDestino(ficha, valorDado, jugador);
    bool destinoEnAnillo = (destino >= 1 && destino <= CASILLAS_ANILLO);
    if (!destinoEnAnillo) return true; // pasillo o meta siempre es seguro
    
    return !casillaTieneBarrera(tablero, destino);
}

// Seleccion por prioridad descendente
static int buscarFichaQueCumple(const EstadoTablero* tablero, int jugador, int valorDado,
    bool (*criterio)(const DatosFicha* , int, int, const EstadoTablero* )) {
    int mejorFicha = -1;
    int mejorPosicion = -1;
    for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
        const DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];
        if (!criterio(ficha, valorDado, jugador, tablero))  continue;
        
        // Entre candidatos, priorizar la mas adelantada en el tablero
        if (ficha->posicion > mejorPosicion) {
            mejorPosicion = ficha->posicion;
            mejorFicha = indiceFicha;
        }
    }
    return mejorFicha;
}

// Adaptadores para criterios sin tablero o sin valorDado
static bool criterioAlcanzarMeta(const DatosFicha* f, int valorDado, int jugador, const EstadoTablero* t) {
    (void)t;
    return puedeAlcanzarMeta(f, valorDado, jugador);
}

static bool criterioSalirDeBase(const DatosFicha* f, int valorDado, int jugador, const EstadoTablero* t) {
    (void)jugador;
    (void)t;
    return puedeSalirDeBase(f, valorDado);
}

int elegirMejorFicha(const EstadoTablero* tablero, int jugador, int valorDado) {
    int candidato = -1;

    candidato = buscarFichaQueCumple(tablero, jugador, valorDado, criterioAlcanzarMeta);
    if (candidato != -1) return candidato;

    candidato = buscarFichaQueCumple(tablero, jugador, valorDado, puedeCapturarRival);
    if (candidato != -1) return candidato;

    candidato = buscarFichaQueCumple(tablero, jugador, valorDado, puedeFormarBarrera);
    if (candidato != -1) return candidato;

    candidato = buscarFichaQueCumple(tablero, jugador, valorDado, criterioSalirDeBase);
    if (candidato != -1) return candidato;

    candidato = buscarFichaQueCumple(tablero, jugador, valorDado, puedeAvanzarSeguro);
    return candidato;
}
