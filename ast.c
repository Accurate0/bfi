#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ast.h"
#include "io.h"
#include "list.h"
#include "util.h"

__attribute__((optimize("unroll-loops")))
bool ast_char_is_valid(int8_t c)
{
    for(int i = INCREMENT_PTR; i < BF_COMMAND_END; i++) {
        if(c == BF_CHARACTERS[i])
            return true;
    }

    return false;
}

static ast_command_t ast_char_to_command(int8_t c)
{
    switch(c) {
        case '>':
            return INCREMENT_PTR;
        case '<':
            return DECREMENT_PTR;

        case '+':
            return INCREMENT_DATA;
        case '-':
            return DECREMENT_DATA;

        case '.':
            return OUTPUT_BYTE;
        case ',':
            return INPUT_BYTE;

        case '[':
            return LOOP_START;
        case ']':
            return LOOP_END;

        default:
            return -1;
    }
}

static ast_expr_t ast_command_to_expr(ast_command_t c)
{
    switch(c) {
        case INCREMENT_DATA:
        case DECREMENT_DATA:
            return DATA;

        case INCREMENT_PTR:
        case DECREMENT_PTR:
            return PTR;

        case OUTPUT_BYTE:
        case INPUT_BYTE:
            return IO;

        case LOOP_START:
        case LOOP_END:
            return LOOP;

        default:
            return -1;
    }
}

void ast_free_node(void *node)
{
    node_t *n = node;

    if(n->type == ROOT || n->type == LOOP) {
        list_free(n->children, ast_free_node);
    }

    free(n);
}

void ast_free(bf_t *bf)
{
    ast_free_node(bf->root);
    free(bf);
}

bf_t* ast_init(void)
{
    bf_t *bf_prog = malloc(sizeof(bf_t));
    bf_prog->root = malloc(sizeof(node_t));
    bf_prog->count = 1;

    node_t *current = bf_prog->root;
    current->parent = NULL;
    current->type = ROOT;
    current->count = 0;
    current->children = list_init();

    node_t *tmp;
    int8_t c;
    while((c = io_read_next()) != EOF) {
        ast_command_t command = ast_char_to_command(c);
        if(command != LOOP_END) {
            tmp = malloc(sizeof(node_t));
            tmp->count = 1;
            tmp->parent = current;
        }

        if(command != LOOP_START && command != LOOP_END) {
            while(c == io_peek()) {
                tmp->count++;
                (void)io_read_next();
            }
        }

        switch(command) {
            case INCREMENT_PTR:
            case DECREMENT_PTR:
            case OUTPUT_BYTE:
            case INPUT_BYTE:
            case INCREMENT_DATA:
            case DECREMENT_DATA: {
                tmp->type = ast_command_to_expr(command);
                tmp->value = command;
                list_add_end(current->children, tmp);
                current->count++;
            } break;
            case LOOP_START: {
                tmp->type = LOOP;
                tmp->children = list_init();
                list_add_end(current->children, tmp);
                current->count++;
                current = tmp;
            } break;
            case LOOP_END: {
                current = current->parent;
            } break;

            default:
                die("unreachable statement -> %s:%d", __FILE__, __LINE__);
        }
    }

    if(current != bf_prog->root) {
        warn("unterminated loop detected");
    }

    return bf_prog;
}
