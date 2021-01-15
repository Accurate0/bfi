#pragma once
#include <stdlib.h>

typedef struct {
    char *str;
    size_t size;
    size_t alloc;
} str_t;

str_t* str_init(size_t initial);
void str_append(str_t *this, const char *str);
void str_free(str_t *this);
