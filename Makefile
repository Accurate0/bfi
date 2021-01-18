CC=gcc
CFLAGS=-Wall -std=c99 -Wextra
LDFLAGS=
TARGET=bfi

SOURCES=$(wildcard src/*.c)
OBJ=${SOURCES:.c=.o}

ifdef DEBUG
	CFLAGS+=-ggdb3 -DDEBUG
else
	CFLAGS+=-O2
endif

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)
