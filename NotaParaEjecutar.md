# Nota de ejecucion
## Resumen
- Usar make rebuild && run
- Expandir la terminal de forma que todo el tablero y la tabla inferior sean visibles por completo

## Makefile
Como se indico se agrega el Makefile correspondiente, por los .log y sockets es mejor hacer:
```
make rebuild && run
```
Para evitar errores

## Terminal
El tablero es medianamente grande y usamos ANSI tambien para recolocar el puntero en cada iteracion, asi que si no se expande
la terminal lo suficiente para poder verla por completo, se pueden empezar a ver bugs visuales de renderizado