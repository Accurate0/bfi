#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "io.h"
#include "list.h"
#include "util.h"

#define MEM_SIZE 30000

static uint32_t BF_DATA[MEM_SIZE] = {
    0,
};
static int32_t BF_PTR = 0;

void interpreter_run(node_t *node) {
  static void *cmd_goto_table[] = {
      &&CMD_INCREMENT_PTR,
      &&CMD_DECREMENT_PTR,
      &&CMD_INCREMENT_DATA,
      &&CMD_DECREMENT_DATA,
      &&CMD_OUTPUT_BYTE,
      &&CMD_INPUT_BYTE,
      NULL,
      NULL,
      &&CMD_OPT_CLEAR,
      &&CMD_OPT_SCAN_LEFT,
      &&CMD_OPT_SCAN_RIGHT,
  };

  // ? apparently a 2nd computed goto is disastrous
  // ? potentially griefing branch prediction

  LIST_FOREACH(node->children, c) {
    node_t *node = c->value;
    uint32_t count = node->count;
    // fprintf(stderr, "count: %d\n", node->count);
    // fprintf(stderr, "type: %d\n", node->type);
    // fprintf(stderr, "value: %d\n", node->value);
    // fprintf(stderr, "ptr: %d\n", BF_PTR);
    ast_command_t value = node->value;

    switch (node->type) {
    case EXPR_LOOP:
      while (BF_DATA[BF_PTR])
        interpreter_run(node);
      continue;

    case EXPR_OPT:
    case EXPR_PTR:
    case EXPR_DATA:
    case EXPR_IO:
      goto *cmd_goto_table[value];

    CMD_INCREMENT_PTR:
      BF_PTR += count;
      continue;

    CMD_DECREMENT_PTR:
      BF_PTR -= count;
      continue;

    CMD_INCREMENT_DATA:
      BF_DATA[BF_PTR] += count;
      continue;

    CMD_DECREMENT_DATA:
      BF_DATA[BF_PTR] -= count;
      continue;

    CMD_OUTPUT_BYTE:
      for (unsigned int i = 0; i < count; i++) {
        printf("%c", BF_DATA[BF_PTR]);
      }
      continue;

    CMD_INPUT_BYTE:
      for (unsigned int i = 0; i < count; i++) {
        BF_DATA[BF_PTR] = fgetc(stdin);
      }
      continue;

    CMD_OPT_CLEAR:
      BF_DATA[BF_PTR] = 0;
      continue;

    CMD_OPT_SCAN_LEFT:
      BF_PTR -= (uint32_t)((void *)(BF_DATA + BF_PTR) -
                           memrchr(BF_DATA, 0, BF_PTR + 1));
      continue;

    CMD_OPT_SCAN_RIGHT:
      BF_PTR += (uint32_t)(memchr(BF_DATA + BF_PTR, 0, MEM_SIZE - BF_PTR) -
                           (void *)(BF_DATA + BF_PTR));
      continue;

    default:
      UNREACHABLE();
    }
  }
}
