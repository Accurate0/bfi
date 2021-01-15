#include <stdlib.h>
#include <string.h>

#include "str.h"

str_t* str_init(size_t initial)
{
    str_t *ret = malloc(sizeof(str_t));
    ret->str = malloc(sizeof(char) * (initial + 1));
    ret->size = 0;
    ret->alloc = 100;
    ret->str[0] = '\0';

    return ret;
}

void str_append(str_t *this, const char *str)
{
    int len = strlen(str);

    if(len + this->size >= this->alloc) {
        this->str = realloc(this->str, sizeof(char) * (this->alloc + len + 1));
    }

    strncat(this->str, str, len);
}

void str_free(str_t *this)
{
    free(this->str);
    free(this);
}
