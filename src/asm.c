#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

asm_t *asm_init() {
  asm_t *ret = malloc(sizeof(asm_t));
  ret->output = malloc(sizeof(uint8_t) * (100));
  ret->size = 0;
  ret->alloc = 100;

  return ret;
}

void asm_emit8(asm_t *this, uint8_t value) {
  if (this->size >= this->alloc) {
    this->alloc += 100;
    this->output = realloc(this->output, sizeof(uint8_t) * (this->alloc));
  }

  this->output[this->size++] = value;
}

void asm_emit32(asm_t *this, uint32_t value) {
  if (this->size >= this->alloc) {
    this->alloc += 100;
    this->output = realloc(this->output, sizeof(uint8_t) * (this->alloc));
  }

  this->output[this->size++] = (value >> 0) & 0xff;
  this->output[this->size++] = (value >> 8) & 0xff;
  this->output[this->size++] = (value >> 16) & 0xff;
  this->output[this->size++] = (value >> 24) & 0xff;
}

void asm_free(asm_t *this) {
  free(this->output);
  free(this);
}
