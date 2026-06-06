# Proyecto/Tarea Programada #2 - Sistemas Operativos

## Configuracion

El archivo `include/config.h` contiene los parametros principales que definen como funciona la simulacion.

* **Ajustes configurables:** Al inicio del archivo hay una seccion de parametros que se pueden editar. Estos valores permiten ajustar la dinamica de la partida segun se necesite.
* **Recomendacion de tiempo:** Se recomienda mantener el *delay* entre 100 y 400. Si se pone un valor muy alto, la partida se alarga demasiado y se vuelve pesada; si es muy bajo, no se alcanza a ver lo que sucede.
* **Advertencia sobre Macros:** A excepcion de los parametros del inicio, **no se deben modificar el resto de las macros** del archivo. Esas macros estan ahi para evitar el *hardcoding* de constantes del sistema, por lo que tocarlas podria causar errores o inestabilidad en el juego.

## Ejecucion

### Comandos de compilacion

Para asegurar un entorno limpio antes de cada ejecucion, usar:

```bash
make rebuild && run

```

**¿Por que usar este comando?**
Las ejecuciones dejan rastros en el sistema dependiendo de los macros y, ante cualquier error inesperado, pueden quedar residuos en memoria o en el sistema de archivos. Este comando limpia los archivos `.log`, sockets y recursos IPC, evitando conflictos entre ejecuciones.

### Requisitos de visualizacion

Es necesario **expandir la terminal** hasta que el tablero de juego y la tabla de estadisticas inferior se vean completos.

**¿Por que hacer esto?**
El tablero es grande y utiliza codigos ANSI para mover el puntero de impresion en cada paso. Si la terminal es muy pequeña para mostrar todo el contenido, el renderizado del ASCII Art se vera cortado o con errores visuales.