#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "asm.h"
#include "ast.h"
#include "jit.h"
#include "util.h"

typedef void (*compiled_function)(uint64_t *data);
static uint64_t BF_DATA[30000] = {
    0,
};

#define CLOBBER_REGISTERS(args...)                                             \
  do {                                                                         \
    __asm__ volatile("" : : : args);                                           \
  } while (0)

static void jit_generate_code(node_t *node, asm_t *assembler,
                              bool should_return) {
  LIST_FOREACH(node->children, c) {
    node_t *node = c->value;
    uint32_t count = node->count;

    // RDI is data
    switch (node->type) {
    case EXPR_PTR:
    case EXPR_DATA:
    case EXPR_IO:
    case EXPR_OPT: {
      switch (node->value) {
      case CMD_INCREMENT_PTR:
        // add edi, count
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x81);
        asm_emit8(assembler, 0xC7);
        asm_emit32(assembler, count * sizeof(int64_t));
        break;

      case CMD_DECREMENT_PTR:
        // sub edi, count
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x81);
        asm_emit8(assembler, 0xEF);
        asm_emit32(assembler, count * sizeof(int64_t));
        break;

      case CMD_INCREMENT_DATA:
        // add qword [rdi], count
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x81);
        asm_emit8(assembler, 0x07);
        asm_emit32(assembler, count);
        break;

      case CMD_DECREMENT_DATA:
        // sub qword [rdi], count
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x81);
        asm_emit8(assembler, 0x2F);
        asm_emit32(assembler, count);
        break;

      case CMD_OUTPUT_BYTE:
        // mov rbx, rdi
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x89);
        asm_emit8(assembler, 0xFB);

        // mov eax, 1
        asm_emit8(assembler, 0xB8);
        asm_emit32(assembler, 1);

        // mov edi, 1
        asm_emit8(assembler, 0xBF);
        asm_emit32(assembler, 1);

        // lea rsi, rbx
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x8D);
        asm_emit8(assembler, 0x33);

        // mov edx, 1
        asm_emit8(assembler, 0xBA);
        asm_emit32(assembler, 1);

        // FIXME: syscall in a loop lmao
        for (uint32_t i = 0; i < count; i++) {
          // syscall
          asm_emit8(assembler, 0x0F);
          asm_emit8(assembler, 0x05);
        }

        // mov rdi, rbx
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x89);
        asm_emit8(assembler, 0xDF);
        break;

      case CMD_INPUT_BYTE:
        // mov rbx, rdi
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x89);
        asm_emit8(assembler, 0xFB);

        // mov eax, 1
        asm_emit8(assembler, 0xB8);
        asm_emit32(assembler, 0);

        // mov edi, 1
        asm_emit8(assembler, 0xBF);
        asm_emit32(assembler, 1);

        // lea rsi, rbx
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x8D);
        asm_emit8(assembler, 0x33);

        // mov edx, 1
        asm_emit8(assembler, 0xBA);
        asm_emit32(assembler, 1);

        // FIXME: syscall in a loop lmao
        for (uint32_t i = 0; i < count; i++) {
          // syscall
          asm_emit8(assembler, 0x0F);
          asm_emit8(assembler, 0x05);
        }

        // mov rdi, rbx
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0x89);
        asm_emit8(assembler, 0xDF);
        break;

      case CMD_OPT_CLEAR:
        // mov qword [rdi], 0x0
        asm_emit8(assembler, 0x48);
        asm_emit8(assembler, 0xC7);
        asm_emit8(assembler, 0x07);
        asm_emit32(assembler, 0x00);
        break;

      case CMD_OPT_SCAN_LEFT:
        TODO();
        break;
      case CMD_OPT_SCAN_RIGHT:
        TODO();
        break;

      default:
        UNREACHABLE();
      }
    } break;

    case EXPR_LOOP: {
      asm_t *loop_body = asm_init();
      jit_generate_code(node, loop_body, false);
      size_t start_of_loop = assembler->size;
      // mov rcx,[rdi]
      asm_emit8(assembler, 0x48);
      asm_emit8(assembler, 0x8B);
      asm_emit8(assembler, 0x0F);

      // test rcx, rcx
      asm_emit8(assembler, 0x48);
      asm_emit8(assembler, 0x85);
      asm_emit8(assembler, 0xC9);

      // jz
      asm_emit8(assembler, 0x0F);
      asm_emit8(assembler, 0x84);
      asm_emit32(assembler, loop_body->size + 6);

      asm_copy_from(assembler, loop_body);

      // mov rcx,[rdi]
      asm_emit8(assembler, 0x48);
      asm_emit8(assembler, 0x8B);
      asm_emit8(assembler, 0x0F);

      // test rcx, rcx
      asm_emit8(assembler, 0x48);
      asm_emit8(assembler, 0x85);
      asm_emit8(assembler, 0xC9);

      // jnz
      asm_emit8(assembler, 0x0F);
      asm_emit8(assembler, 0x85);
      asm_emit32(assembler, start_of_loop - (assembler->size + 4));

      asm_free(loop_body);
    } break;

    default:
      UNREACHABLE();
    }
  }

  // ret
  if (should_return) {
    asm_emit8(assembler, 0xC3);
  }
}

void jit_run(node_t *node) {
  asm_t *assembler = asm_init();
  jit_generate_code(node, assembler, true);
  size_t size = assembler->size;

  void *ptr =
      mmap(NULL, size * sizeof(uint8_t), PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memcpy(ptr, assembler->output, size * sizeof(uint8_t));

#if DEBUG
  write(STDERR_FILENO, ptr, size * sizeof(uint8_t));
  fflush(stdout);
#endif

  CLOBBER_REGISTERS("rdi", "rbx", "rax", "rsi", "rcx");
  ((compiled_function)ptr)(BF_DATA);

  asm_free(assembler);
  munmap(ptr, size * sizeof(uint8_t));
}
