#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "io.h"
#include "list.h"
#include "util.h"

typedef void (*interpreter_command)(uint32_t);
static uint8_t BF_DATA[30000] = {0,};
static int32_t BF_PTR = 0;

void increment_data(uint32_t count) {
    BF_DATA[BF_PTR] += count;
}

void decrement_data(uint32_t count) {
    BF_DATA[BF_PTR] -= count;
}

void increment_ptr(uint32_t count) {
    BF_PTR += count;
}

void decrement_ptr(uint32_t count) {
    BF_PTR -= count;
}

// input and output can be repeated but
// i don't think its the case often
void output_byte(uint32_t count) {
    for(unsigned int i = 0; i < count; i++) {
        printf("%c", BF_DATA[BF_PTR]);
    }
}

void input_byte(uint32_t count) {
    for(unsigned int i = 0; i < count; i++) {
        BF_DATA[BF_PTR] = fgetc(stdin);
    }
}

static const interpreter_command BF_FUNCTIONS[CMD_END] = {
    increment_ptr,
    decrement_ptr,
    increment_data,
    decrement_data,
    output_byte,
    input_byte,
    NULL,
    NULL,
};

void clear_loop(uint32_t count) {
    (void)count;
    BF_DATA[BF_PTR] = 0;
}

// SCAN LEFT AND RIGHT CAN ONLY WORK ON CHAR SIZED CELLS
void scan_left(uint32_t count) {
    (void)count;
    BF_PTR -= (uint32_t)((void *)(BF_DATA + BF_PTR) - memrchr(BF_DATA, 0, BF_PTR + 1));
}

void scan_right(uint32_t count) {
    (void)count;
    BF_PTR += (uint32_t)(memchr(BF_DATA + BF_PTR, 0, sizeof(BF_DATA)) - (void *)(BF_DATA + BF_PTR));
}

static const interpreter_command BF_OPT_FUNCTIONS[CMD_OPT_END - CMD_OPT_BEGIN - 1] = {
    NULL,
    clear_loop,
    scan_left,
    scan_right,
};


void interpreter_run(node_t *node)
{
    LIST_FOREACH(node->children, c) {
        node_t *node = c->value;

        switch(node->type) {
            case EXPR_PTR:
            case EXPR_DATA:
            case EXPR_IO: {
                BF_FUNCTIONS[node->value](node->count);
            } break;

            case EXPR_LOOP: {
                while(BF_DATA[BF_PTR])
                    interpreter_run(node);
            } break;

            case EXPR_OPT: {
                BF_OPT_FUNCTIONS[node->value - CMD_OPT_BEGIN - 1](node->count);
            } break;

            default:
                UNREACHABLE();
        }
    }
}
