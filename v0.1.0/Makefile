CC=		cc

STDFLAGS=	-std=c11 -pedantic
OFLAGS=		-O2
WFLAGS=		-Wall \
		-Wextra \
		-Wconversion \
		-Wfloat-equal \
		-Wshadow \
		-Wpointer-arith \
		-Wcast-align \
		-Wstrict-prototypes \
		-Wstrict-overflow=5 \
		-Wwrite-strings \
		-Wcast-qual \
		-Wundef \
		-Wunreachable-code \
		-Wuninitialized \
		-Wformat=2 \
		-Wno-format-nonliteral
FFLAGS=		-ftrapv
EXTRAFLAGS=	-Werror \
		-DNDEBUG
CFLAGS=		$(STDFLAGS) $(OFLAGS) $(WFLAGS) $(FFLAGS) $(EXTRAFLAGS)
LDFLAGS=

SRC=		main.c
BIN=		pop

.PHONY: all debug test clean

$(BIN): $(SRC)
	$(CC) -o $(BIN) $(CFLAGS) $(SRC) $(LDFLAGS)

all: $(BIN)

debug: OFLAGS= -Og
debug: EXTRAFLAGS= -g -fno-omit-frame-pointer
debug: all

clean:
	rm -f $(BIN)
