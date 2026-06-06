# Compilador y flags básicos
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
# -D_GNU_SOURCE porque usamos algunas cosas de Linux que no son POSIX puro
CFLAGS += -D_GNU_SOURCE

# Librerías que necesitamos linkear
LIBS = -lpthread -lrt

# Directorios del proyecto
SRC = src
INC = include
OBJ = obj
BIN = bin
EXE = $(BIN)/parchis

# Busco todos los .c en src/ y genero la lista de .o correspondientes
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SOURCES))

# Regla principal: compilar todo
all: dirs $(EXE)

# Crear carpetas si no existen
dirs:
	@mkdir -p $(BIN) $(OBJ)

# Linkeo final
$(EXE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Compilado: $@"

# Compilar cada .c a .o
# -MMD genera las dependencias automáticamente (para los headers)
$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -MMD -c $< -o $@

# Incluir dependencias de headers (generadas por -MMD)
-include $(OBJECTS:.o=.d)

# Limpiar archivos temporales y el binario
clean:
	rm -rf $(OBJ) $(BIN) debug_parchis.log
	@echo "Limpio."

# Borrar recursos IPC que a veces se quedan colgados si el programa crashea
clean-ipc:
	-ipcrm -M 0x50415243 2>/dev/null || true
	-ipcrm -Q 0x50415244 2>/dev/null || true
	-rm -f /tmp/parchis_arbitro.sock
	@echo "IPC limpio."

# Recompilar desde cero
rebuild: clean all

# Compilar y ejecutar, limpiando IPC antes por si acaso
run: all
	-$(MAKE) clean-ipc
	./$(EXE)

.PHONY: all dirs clean clean-ipc rebuild run