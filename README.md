# Proyecto/Tarea Programada #2 - Sistemas Operativos

# Ejecucion
- Usar make rebuild && run
- Expandir la terminal de forma que todo el tablero y la tabla inferior sean visibles por completo
A que se debe esto?

**Makefile**
Las ejecuciones dejan ciertos rastros depende de los macros elegidos y siempre cabe la posibilidad de que por un error del
sistema operativo queden residuos, durante el testeo paso y por ello es mejor usar el comando
```
make rebuild && run
```
Para evitar errores ya que limpia .logs y sockets

**Terminal**
El tablero es medianamente grande y usamos ANSI tambien para recolocar el puntero en cada iteracion, asi que si no se expande
la terminal lo suficiente para poder verla por completo, se pueden empezar a ver bugs visuales de renderizado