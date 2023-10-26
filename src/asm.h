#pragma once
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint8_t *output;
  size_t size;
  size_t alloc;
} asm_t;

asm_t *asm_init();
void asm_emit8(asm_t *asm_this, uint8_t value);
void asm_emit16(asm_t *asm_this, uint16_t value);
void asm_emit32(asm_t *asm_this, uint32_t value);
void asm_emit64(asm_t *asm_this, uint64_t value);
void asm_copy_from(asm_t *asm_this, asm_t *other);
void asm_free(asm_t *asm_this);
