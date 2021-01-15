#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "ast.h"

static FILE *f = NULL;
static int8_t current = -1;

void io_open_file(const char *path)
{
    f = fopen(path, "r");
    if(!f) {
        die("%s", strerror(errno));
    }
}

void io_set_file(FILE *file)
{
    f = file;
}

int8_t io_peek()
{
    return current;
}

static int8_t io_find_next()
{
    int8_t c;
    while((c = fgetc(f)) != EOF && !ast_char_is_valid(c)) {}
    return c;
}

int8_t io_read_next()
{
    if(!f)
        return -1;

    if(current == -1)
        current = io_find_next();

    int8_t c = current;
    current = io_find_next();

    return c;
}


void io_close_file()
{
    if(f)
        fclose(f);
}
