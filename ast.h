#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "list.h"

typedef enum BF_COMMANDS {
    INCREMENT_PTR = 0,
    DECREMENT_PTR,
    INCREMENT_DATA,
    DECREMENT_DATA,
    OUTPUT_BYTE,
    INPUT_BYTE,
    LOOP_START,
    LOOP_END,
    BF_COMMAND_END,
} ast_command_t;

typedef enum {
    ROOT,
    DATA,
    PTR,
    LOOP,
    IO,
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

static const uint8_t BF_CHARACTERS[BF_COMMAND_END] = {
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
bf_t* ast_init();
void ast_free(bf_t *bf);
