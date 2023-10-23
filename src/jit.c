#include "asm.h"
#include <string.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "ast.h"
#include "jit.h"
#include "util.h"

typedef void (*compiled_function)(int32_t *data);
static int32_t BF_DATA[30000] = {
    0,
};

static size_t jit_generate_code(node_t *node, asm_t *assembler, size_t size) {
  LIST_FOREACH(node->children, c) {
    node_t *node = c->value;

    // RDI is data
    switch (node->type) {
    case EXPR_PTR:
    case EXPR_DATA:
    case EXPR_IO:
    case EXPR_OPT: {
      switch (node->value) {
      case CMD_INCREMENT_PTR:
        // inc rdi
        asm_emit8(assembler, 0xFF);
        asm_emit8(assembler, 0xC7);
        size += 2;
        break;

      case CMD_DECREMENT_PTR:
        // dec rdi
        asm_emit8(assembler, 0xFF);
        asm_emit8(assembler, 0xCF);
        size += 2;
        break;

      case CMD_INCREMENT_DATA:
        // inc [rdi]
        asm_emit8(assembler, 0xFF);
        asm_emit8(assembler, 0x07);
        size += 2;
        break;

      case CMD_DECREMENT_DATA:
        // dec [rdi]
        asm_emit8(assembler, 0xFF);
        asm_emit8(assembler, 0x0F);
        size += 2;
        break;

      case CMD_OUTPUT_BYTE:
        // need to preserve rdi to syscall
        break;

      case CMD_INPUT_BYTE:
        break;

      case CMD_OPT_CLEAR:
        // mov dword [rdi], 0x0
        asm_emit8(assembler, 0xC7);
        asm_emit8(assembler, 0x07);
        asm_emit32(assembler, 0x00);
        size += 4;
        break;

      default:
        UNREACHABLE();
      }
    } break;

    case EXPR_LOOP: {
      // loop is something like
      // while(*RDI != 0) = [
      // when RDI == 0 JMP to ] + 1
      // else JMP to [ + 1
      // execute inner code ...
      // jit_generate_code
      // jmp back to start ]

      // cmp [register] 0
      // size of code = recursively generate code
      // je current_addr + size_of_code
      // continue otherwise
      // at end jmp back to beginning
    } break;

    default:
      UNREACHABLE();
    }
  }

  asm_emit8(assembler, 0xC3);
  size += 1;

  return size;
}

void jit_run(node_t *node) {
  asm_t *assembler = asm_init();
  size_t size = jit_generate_code(node, assembler, 0);

  void *ptr =
      mmap(NULL, size * sizeof(uint8_t), PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memcpy(ptr, assembler->output, size * sizeof(uint8_t));
  write(STDOUT_FILENO, ptr, size * sizeof(uint8_t));
  fflush(stdout);

  ((compiled_function)ptr)(BF_DATA);

  asm_free(assembler);
  munmap(ptr, size);
}
