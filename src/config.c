#include "config.h"

ConfiguracionSimulador configuracionGlobal;

void inicializarConfiguracion(bool activarLogs, bool printInfo, int delayMs) {
    configuracionGlobal.activarLogs = activarLogs;
    configuracionGlobal.printInfo = printInfo;
    configuracionGlobal.delayMilisegundos = delayMs;
}
