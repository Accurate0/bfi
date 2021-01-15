#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "list.h"

typedef enum BF_COMMANDS {
    CMD_INCREMENT_PTR = 0,
    CMD_DECREMENT_PTR,
    CMD_INCREMENT_DATA,
    CMD_DECREMENT_DATA,
    CMD_OUTPUT_BYTE,
    CMD_INPUT_BYTE,
    CMD_LOOP_START,
    CMD_LOOP_END,
    CMD_END,
    CMD_OPT_BEGIN,
    CMD_OPT_NONE,
    CMD_OPT_CLEAR,
    CMD_OPT_END,
} ast_command_t;

typedef enum {
    EXPR_ROOT,
    EXPR_DATA,
    EXPR_PTR,
    EXPR_LOOP,
    EXPR_IO,
    EXPR_OPT,
} ast_expr_t;

typedef struct node_t {
    ast_expr_t type;
    uint32_t count;
    struct node_t *parent;
    union {
        list_t *children;
        ast_command_t value;
    };
} node_t;

typedef struct {
    node_t *root;
    uint32_t count;
} bf_t;

static const uint8_t BF_CHARACTERS[CMD_END] = {
    '>',
    '<',
    '+',
    '-',
    '.',
    ',',
    '[',
    ']'
};

bool ast_char_is_valid(int8_t c);
bf_t* ast_init(bool optimisations);
void ast_optimise(node_t *root);
void ast_stats(bf_t *prog);
void ast_free(bf_t *bf);
