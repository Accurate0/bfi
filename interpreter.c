#include <stdio.h>

#include "ast.h"
#include "io.h"
#include "list.h"
#include "util.h"

typedef void (*interpreter_command)(uint32_t);
static int32_t BF_DATA[30000] = {0,};
static int32_t *BF_PTR = BF_DATA;

void increment_data(uint32_t count) {
    *BF_PTR += count;
}

void decrement_data(uint32_t count) {
    *BF_PTR -= count;
}

void increment_ptr(uint32_t count) {
    BF_PTR += count;
}

void decrement_ptr(uint32_t count) {
    BF_PTR -= count;
}

void output_byte(uint32_t count) {
    for(unsigned int i = 0; i < count; i++) {
        printf("%c", *BF_PTR);
    }
}

void input_byte(uint32_t count) {
    for(unsigned int i = 0; i < count; i++) {
        *BF_PTR = fgetc(stdin);
    }
}

static const interpreter_command BF_FUNCTIONS[BF_COMMAND_END] = {
    increment_ptr,
    decrement_ptr,
    increment_data,
    decrement_data,
    output_byte,
    input_byte,
    NULL,
    NULL,
};


void interpreter_run(node_t *node)
{
    LIST_FOREACH(node->children, c) {
        node_t *node = c->value;

        switch(node->type) {
            case ROOT:
            case PTR:
            case DATA:
            case IO: {
                BF_FUNCTIONS[node->value](node->count);
            } break;

            case LOOP: {
                while(*BF_PTR)
                    interpreter_run(node);
            } break;
        }
    }
}
