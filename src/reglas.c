#include "reglas.h"
#include "config.h"
#include "log.h"
#include <stdio.h>

const int SALIDA_JUGADOR[TOTAL_JUGADORES] = {
    SALIDA_ROJO, 
    SALIDA_VERDE,
    SALIDA_AMARILLO,
    SALIDA_AZUL
};

const int ENTRADA_PASILLO[TOTAL_JUGADORES] = {
    ENTRADA_PASILLO_ROJO,
    ENTRADA_PASILLO_VERDE,
    ENTRADA_PASILLO_AMARILLO,
    ENTRADA_PASILLO_AZUL
};


// Consultas base
bool fichaEstaEnJuego(const DatosFicha* ficha) {
    return ficha->estado == FICHA_EN_ANILLO || ficha->estado == FICHA_EN_PASILLO;
}

bool fichaLlegoAMeta(const DatosFicha* ficha) {
    return ficha->estado == FICHA_EN_META;
}

int avanzarEnAnillo(int posicionActual, int pasos) {
    return ((posicionActual - 1 + pasos) % CASILLAS_ANILLO) + 1;
}

bool posicionEntraAlPasillo(int posicion, int pasos, int jugador) {
    int entradaPasillo = ENTRADA_PASILLO[jugador];
    // Detectamos si al avanzar 'pasos' desde 'posicion' se cruza la entrada
    for (int paso = 1; paso <= pasos; paso++) {
        if (avanzarEnAnillo(posicion, paso) == entradaPasillo) {
            return true;
        }
    }
    return false;
}

bool rivalTieneBarrera(const EstadoTablero* tablero, int jugador, int casilla) {
    return casillaTieneBarrera(tablero, casilla) && contarFichasEnCasilla(tablero, casilla, jugador) == 0;
}

int calcularPosicionEnPasillo(const DatosFicha* ficha, int valorDado, int jugador) {
    if (ficha->estado == FICHA_EN_PASILLO) return ficha->posicion + valorDado;
    
    // Calcula cuantos pasos quedan despues de cruzar la entrada
    int entradaPasillo = ENTRADA_PASILLO[jugador];
    int pasosHastaEntrada = 0;
    int posicionTemp = ficha->posicion;
    while (avanzarEnAnillo(posicionTemp, 1) != entradaPasillo) {
        posicionTemp = avanzarEnAnillo(posicionTemp, 1);
        pasosHastaEntrada++;
    }
    pasosHastaEntrada++;

    int pasosEnPasillo = valorDado - pasosHastaEntrada;
    return INICIO_PASILLO + pasosEnPasillo;
}

int calcularPosicionDestino(const DatosFicha* ficha, int valorDado, int jugador) {
    if (ficha->estado == FICHA_EN_BASE) return SALIDA_JUGADOR[jugador];
    if (ficha->estado == FICHA_EN_PASILLO) return ficha->posicion + valorDado;

    bool entraAlPasillo = posicionEntraAlPasillo(ficha->posicion, valorDado, jugador);
    if (entraAlPasillo) return calcularPosicionEnPasillo(ficha, valorDado, jugador);

    return avanzarEnAnillo(ficha->posicion, valorDado);
}
 
static bool validarEstadoInicial(const DatosFicha* ficha, int valorDado, int destino) {
    if (ficha->estado == FICHA_EN_META) return false;
    if (ficha->estado == FICHA_EN_BASE && valorDado != 6) return false;

    return destino <= CASILLA_META;
}

static bool destinoBloqueadoPorSegura(const EstadoTablero* tablero, int destino, int jugador) {
    for (int rival = 0; rival < TOTAL_JUGADORES; rival++) {
        if (rival == jugador) continue;

        bool esSegura = casillaEsSegura(destino, rival);
        bool ocupada = contarFichasEnCasilla(tablero, destino, rival) > 0;
        if (esSegura && ocupada) return true;
    }

    return false;
}

static bool validarCasillaDestino(const EstadoTablero* tablero, int destino, int jugador) {
    bool destinoEnAnillo = destino >= 1 && destino <= CASILLAS_ANILLO;
    if (!destinoEnAnillo) return true;

    if (casillaTieneBarrera(tablero, destino)) {
        bool barreraPropia = contarFichasEnCasilla(tablero, destino, jugador) >= 2;

        if (!barreraPropia) return false;
    }

    return !destinoBloqueadoPorSegura(tablero, destino, jugador);
}

static int obtenerLimiteRecorrido(const DatosFicha* ficha, int valorDado) {
    int jugador = ficha->duenio;

    for (int paso = 1; paso <= valorDado; paso++) {
        if (avanzarEnAnillo(ficha->posicion, paso) == ENTRADA_PASILLO[jugador]) return paso;
    }

    return valorDado - 1;
}

static bool validarBarrerasEnRecorrido(const DatosFicha* ficha, int valorDado, const EstadoTablero* tablero) {
    if (ficha->estado != FICHA_EN_ANILLO) return true;

    int limite = obtenerLimiteRecorrido(ficha, valorDado);

    for (int paso = 1; paso <= limite; paso++) {
        int casilla = avanzarEnAnillo(ficha->posicion, paso);

        if (casillaTieneBarrera(tablero, casilla)) return false;
    }

    return true;
}

bool movimientoEsValido(const DatosFicha* ficha, int valorDado, const EstadoTablero* tablero) {
    int destino = calcularPosicionDestino(ficha, valorDado, ficha->duenio);

    if (!validarEstadoInicial(ficha, valorDado, destino)) return false;
    if (!validarCasillaDestino(tablero, destino, ficha->duenio)) return false;

    return validarBarrerasEnRecorrido(ficha, valorDado, tablero);
}

void devolverFichaABase(EstadoTablero* tablero, int jugador, int indiceFicha) {
    DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];
    ficha->estado = FICHA_EN_BASE;
    ficha->posicion = 0;
}

void sacarFichaDeBase(EstadoTablero* tablero, int jugador, int indiceFicha) {
    int casillaSalida = SALIDA_JUGADOR[jugador];

    if (pthread_mutex_trylock(&tablero->mutexCasilla[casillaSalida]) != 0) return;

    // La casilla de salida es segura para el dueno, pero si hay un rival solo (sin barrera), se lo captura igualmente al salir
    if (rivalTieneBarrera(tablero, jugador, casillaSalida)) {
        pthread_mutex_unlock(&tablero->mutexCasilla[casillaSalida]);
        return;
    }

    intentarCapturarRival(tablero, jugador, casillaSalida);

    DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];
    ficha->estado = FICHA_EN_ANILLO;
    ficha->posicion = casillaSalida;

    pthread_mutex_unlock(&tablero->mutexCasilla[casillaSalida]);
}

bool intentarCapturarRival(EstadoTablero* tablero, int jugadorAtacante, int casilla) {
    int jugadorRival = encontrarRivalEnCasilla(tablero, casilla, jugadorAtacante);
    if (jugadorRival == -1) return false;
    
    // Encontrar cual ficha del rival esta en esa casilla
    for (int indiceFicha = 0; indiceFicha < FICHAS_POR_JUGADOR; indiceFicha++) {
        DatosFicha* ficha = &tablero->fichas[jugadorRival][indiceFicha];
        if (ficha->estado == FICHA_EN_ANILLO && ficha->posicion == casilla) {
            devolverFichaABase(tablero, jugadorRival, indiceFicha);
            tablero->estadisticas[jugadorAtacante].fichasComidas++;
            tablero->estadisticas[jugadorRival].fichasComidaRecibidas++;
            tablero->estadisticas[jugadorAtacante].puntos += PUNTOS_POR_COMER;

            char mensajeEvento[64];
            snprintf(mensajeEvento, sizeof(mensajeEvento), "Come ficha de %s", NOMBRE_JUGADOR[jugadorRival]);
            registrarEvento(jugadorAtacante, indiceFicha, mensajeEvento);

            return true;
        }
    }
    return false;
}

void aplicarMovimiento(EstadoTablero* tablero, int jugador, int indiceFicha, int valorDado) {
    DatosFicha* ficha = &tablero->fichas[jugador][indiceFicha];

   if (ficha->estado == FICHA_EN_BASE) {
        int posicionAntes = ficha->posicion;
        sacarFichaDeBase(tablero, jugador, indiceFicha);
        
        bool fichaSalio = (ficha->estado == FICHA_EN_ANILLO && ficha->posicion != posicionAntes);
        if (fichaSalio) {
            registrarMovimiento(jugador, indiceFicha, valorDado, "Sale al tablero");
        }
        return;
    }
    int posicionDestino = calcularPosicionDestino(ficha, valorDado, jugador);

    if (posicionDestino == CASILLA_META) {
        sem_wait(&tablero->semaforoMeta[jugador]);

        ficha->estado   = FICHA_EN_META;
        ficha->posicion = CASILLA_META;

        tablero->estadisticas[jugador].fichasEnMeta++;
        tablero->estadisticas[jugador].puntos += PUNTOS_POR_LLEGAR_META;

        sem_post(&tablero->semaforoMeta[jugador]);

        registrarEvento(jugador, indiceFicha, "Llega a la meta");

        return;
    }

    bool entraAlPasillo = posicionEntraAlPasillo(ficha->posicion, valorDado, jugador);
    if (entraAlPasillo || ficha->estado == FICHA_EN_PASILLO) {
        sem_wait(&tablero->semaforoPasillo[jugador]);

        ficha->estado   = FICHA_EN_PASILLO;
        ficha->posicion = posicionDestino;

        sem_post(&tablero->semaforoPasillo[jugador]);

        char descripcion[48];
        snprintf(descripcion, sizeof(descripcion), "Avanza en pasillo a casilla %d", posicionDestino);
        registrarMovimiento(jugador, indiceFicha, valorDado, descripcion);

        return;
    }

    // Movimiento en el anillo: adquirir mutex de destino con try-lock.
    // Si no esta disponible, abandonar el intento (evita espera circular).
    if (pthread_mutex_trylock(&tablero->mutexCasilla[posicionDestino]) != 0) return;

    // Re-validar al adquirir el lock. Entre que la IA eligio la casilla y que tomamos el mutex, otro proceso/hilo pudo haberla ocupado. Sin esta
    if (rivalTieneBarrera(tablero, jugador, posicionDestino)) {
        pthread_mutex_unlock(&tablero->mutexCasilla[posicionDestino]);
        return;
    }

    intentarCapturarRival(tablero, jugador, posicionDestino);
    ficha->posicion = posicionDestino;
    ficha->estado = FICHA_EN_ANILLO;

    pthread_mutex_unlock(&tablero->mutexCasilla[posicionDestino]);

    char descripcion[48];
    snprintf(descripcion, sizeof(descripcion), "Se mueve %d espacios", valorDado);
    registrarMovimiento(jugador, indiceFicha, valorDado, descripcion);
}
