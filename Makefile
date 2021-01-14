CC=gcc
CFLAGS=-Wall -std=c99 -Wextra
LDFLAGS=
TARGET=bfi

SOURCES=$(wildcard *.c)
OBJ=${SOURCES:.c=.o}

ifndef DEBUG
	CFLAGS+=-g -DDEBUG -fsanitize=leak
	LDFLAGS+=-llsan
endif

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)
