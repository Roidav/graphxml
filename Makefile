# ===============================
# GraphXML Makefile
# Cross-platform Desktop: Linux / macOS / Windows (MSYS2)
# ===============================

# Compiler
CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Iinclude

# Source files
SRC := src/main.c src/parser.c src/renderer.c src/utils.c
OBJ := $(SRC:.c=.o)

# Output binary
BIN := graphxml

# SDL2 paths (optional override)
SDL_CFLAGS ?= $(shell sdl2-config --cflags)
SDL_LDFLAGS ?= $(shell sdl2-config --libs)

# Libraries
LIBS := -lSDL2_ttf -lm

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS
    CFLAGS += $(SDL_CFLAGS)
    LDFLAGS := $(SDL_LDFLAGS) $(LIBS)
else ifeq ($(UNAME_S),Linux)
    # Linux
    CFLAGS += $(SDL_CFLAGS)
    LDFLAGS := $(SDL_LDFLAGS) $(LIBS)
else ifeq ($(OS),Windows_NT)
    # Windows (MSYS2 / MinGW)
    CFLAGS += -I/mingw64/include/SDL2
    LDFLAGS := -L/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lm
endif

# ===============================
# Rules
# ===============================

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

# Optional: run example
run: $(BIN)
	@echo "Running GraphXML example..."
	./$(BIN) examples/example.graphxml assets/fonts/DejaVuSans.ttf