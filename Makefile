CC=gcc
CFLAGS=-Wall -std=c99 -Wextra
LDFLAGS=
TARGET=bfi

SOURCES=$(wildcard *.c)
OBJ=${SOURCES:.c=.o}

ifdef DEBUG
	CFLAGS+=-g -DDEBUG -fsanitize=leak
	LDFLAGS+=-llsan
else
	CFLAGS+=-O3
endif

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)
