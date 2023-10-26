#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "util.h"

asm_t *asm_init() {
  asm_t *ret = malloc(sizeof(asm_t));
  ret->output = malloc(sizeof(uint8_t) * (100));
  ret->size = 0;
  ret->alloc = 100;

  return ret;
}

static void asm_realloc_if_needed(asm_t *this) {
  // FIXME: -20 to ensure realloc happens before we write into the empty
  // space...
  if (this->size >= this->alloc - 20) {
    this->alloc += 100;
    this->output = realloc(this->output, sizeof(uint8_t) * (this->alloc));
  }
}

void asm_emit8(asm_t *this, uint8_t value) {
  asm_realloc_if_needed(this);

  this->output[this->size++] = value;
}

void asm_emit32(asm_t *this, uint32_t value) {
  asm_realloc_if_needed(this);

  this->output[this->size++] = (value >> 0) & 0xff;
  this->output[this->size++] = (value >> 8) & 0xff;
  this->output[this->size++] = (value >> 16) & 0xff;
  this->output[this->size++] = (value >> 24) & 0xff;
}

void asm_emit64(asm_t *this, uint64_t value) {
  asm_realloc_if_needed(this);

  this->output[this->size++] = (value >> 0) & 0xff;
  this->output[this->size++] = (value >> 8) & 0xff;
  this->output[this->size++] = (value >> 16) & 0xff;
  this->output[this->size++] = (value >> 24) & 0xff;
  this->output[this->size++] = (value >> 32) & 0xff;
  this->output[this->size++] = (value >> 40) & 0xff;
  this->output[this->size++] = (value >> 48) & 0xff;
  this->output[this->size++] = (value >> 56) & 0xff;
}

void asm_emit16(asm_t *this, uint16_t value) {
  asm_realloc_if_needed(this);

  this->output[this->size++] = (value >> 0) & 0xff;
  this->output[this->size++] = (value >> 8) & 0xff;
}

void asm_copy_from(asm_t *this, asm_t *other) {
  if (other->size >= (this->alloc - this->size)) {
    this->alloc += other->size + 100;
    this->output = realloc(this->output, sizeof(uint8_t) * (this->alloc));
  }

  memcpy(this->output + this->size, other->output, other->size);
  this->size += other->size;
}

void asm_free(asm_t *this) {
  free(this->output);
  free(this);
}
