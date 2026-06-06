#include "dado.h"
#include <stdlib.h>
#include <time.h>

void inicializarSemillaAleatoria(int semilla) {
    srand((unsigned int)(time(NULL) ^ semilla));
}

int lanzarDado(void) {
    return (rand() % 6) + 1;
}
