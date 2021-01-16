#pragma once
#include <stdio.h>
#include <stdint.h>

void io_open_file(const char *path);
int8_t io_read_next();
int8_t io_peek();
void io_close_file();
void io_set_file(FILE *file);
