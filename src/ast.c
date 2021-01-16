#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ast.h"
#include "io.h"
#include "list.h"
#include "util.h"

inline bool ast_char_is_valid(int8_t c)
{
    for(int i = CMD_INCREMENT_PTR; i < CMD_END; i++) {
        if(c == BF_CHARACTERS[i])
            return true;
    }

    return false;
}

static inline ast_command_t ast_char_to_command(int8_t c)
{
    switch(c) {
        case '>':
            return CMD_INCREMENT_PTR;
        case '<':
            return CMD_DECREMENT_PTR;

        case '+':
            return CMD_INCREMENT_DATA;
        case '-':
            return CMD_DECREMENT_DATA;

        case '.':
            return CMD_OUTPUT_BYTE;
        case ',':
            return CMD_INPUT_BYTE;

        case '[':
            return CMD_LOOP_START;
        case ']':
            return CMD_LOOP_END;

        default:
            return -1;
    }
}

static inline ast_expr_t ast_command_to_expr(ast_command_t c)
{
    switch(c) {
        case CMD_INCREMENT_DATA:
        case CMD_DECREMENT_DATA:
            return EXPR_DATA;

        case CMD_INCREMENT_PTR:
        case CMD_DECREMENT_PTR:
            return EXPR_PTR;

        case CMD_OUTPUT_BYTE:
        case CMD_INPUT_BYTE:
            return EXPR_IO;

        case CMD_LOOP_START:
        case CMD_LOOP_END:
            return EXPR_LOOP;

        default:
            return -1;
    }
}

void ast_free_node(void *node)
{
    node_t *n = node;

    if(n->type == EXPR_ROOT || n->type == EXPR_LOOP) {
        list_free(n->children, ast_free_node);
    }

    free(n);
}

void ast_free(bf_t *bf)
{
    ast_free_node(bf->root);
    free(bf);
}

static inline void ast_optimise_loop_node(node_t *node, list_t *loop, ast_command_t opt)
{
    switch(opt) {
        case CMD_OPT_SCAN_RIGHT:
        case CMD_OPT_SCAN_LEFT:
        case CMD_OPT_CLEAR: {
            // no longer need the loop
            node->count -= loop->count;
            node->type = EXPR_OPT;
            node->value = opt;
            list_free(loop, ast_free_node);
        } break;

        default:
            UNREACHABLE();
    }
}

void ast_optimise(node_t *root)
{
    ast_command_t optimise = CMD_OPT_NONE;

    LIST_FOREACH(root->children, c) {
        node_t *node = c->value;

        switch(node->type) {
            case EXPR_ROOT:
            case EXPR_PTR:
            case EXPR_DATA:
            case EXPR_IO:
                break;

            case EXPR_LOOP: {
                list_t *loop = node->children;
                if(loop->count == 1) {
                    node_t *first = ((node_t*)loop->head->value);

                    // check for clear loop optimisation potential
                    if(first->type == EXPR_DATA) {
                        if(first->value == CMD_DECREMENT_DATA)
                            optimise = CMD_OPT_CLEAR;
                        else if(first->value == CMD_INCREMENT_DATA)
                            optimise = CMD_OPT_CLEAR;
                    }

                    // figure out why node count is need to prevent
                    // hanoi from going ham
                    if(node->count == 1 && first->type == EXPR_PTR) {
                        if(first->value == CMD_INCREMENT_PTR)
                            optimise = CMD_OPT_SCAN_RIGHT;
                        else if(first->value == CMD_DECREMENT_PTR)
                            optimise = CMD_OPT_SCAN_LEFT;
                    }
                }

                // if an optimisation was detected on this loop
                // optimise it, otherwise check the nodes inside the loop recursively
                if(optimise != CMD_OPT_NONE)
                    ast_optimise_loop_node(node, loop, optimise);
                else
                    ast_optimise(node);

                // reset our optimisation
                optimise = CMD_OPT_NONE;
            } break;

            default:
                UNREACHABLE();
        }
    }
}

bf_t* ast_init(bool optimisations)
{
    bf_t *bf_prog = malloc(sizeof(bf_t));
    bf_prog->root = malloc(sizeof(node_t));
    bf_prog->count = 1;

    node_t *current = bf_prog->root;
    current->parent = NULL;
    current->type = EXPR_ROOT;
    current->count = 0;
    current->children = list_init();

    int8_t c;
    while((c = io_read_next()) != EOF) {
        node_t *tmp;
        ast_command_t command = ast_char_to_command(c);
        if(command != CMD_LOOP_END) {
            tmp = malloc(sizeof(node_t));
            tmp->count = 1;
            tmp->parent = current;
        }

        // if we're optimising, compress +++++ into +x5
        // it's easier to do this when creating the AST
        // also saves on allocations if this is done now
        // instead of during the optimisation pass
        // this alone saves nearly 10 seconds on hanoi.bf
        if(optimisations) {
            if(command != CMD_LOOP_START && command != CMD_LOOP_END) {
                while(c == io_peek()) {
                    tmp->count++;
                    (void)io_read_next();
                }
            }
        }

        switch(command) {
            case CMD_INCREMENT_PTR:
            case CMD_DECREMENT_PTR:
            case CMD_OUTPUT_BYTE:
            case CMD_INPUT_BYTE:
            case CMD_INCREMENT_DATA:
            case CMD_DECREMENT_DATA: {
                tmp->type = ast_command_to_expr(command);
                tmp->value = command;
                list_add_end(current->children, tmp);
                current->count++;
                if(current != bf_prog->root)
                    bf_prog->root->count++;
            } break;
            case CMD_LOOP_START: {
                tmp->type = EXPR_LOOP;
                tmp->children = list_init();
                list_add_end(current->children, tmp);
                current = tmp;
                if(current != bf_prog->root)
                    bf_prog->root->count++;
            } break;
            case CMD_LOOP_END: {
                current = current->parent;
            } break;

            default:
                UNREACHABLE();
        }
    }

    if(current != bf_prog->root) {
        warn("unterminated loop detected");
    }

    return bf_prog;
}

static void ast_print_spaces(int spaces)
{
    for (int i = 0; i < spaces; i++)
    {
        printf(" ");
    }

}

static char* ast_val_to_str(ast_command_t val)
{
    switch(val) {
        case CMD_INCREMENT_PTR:
            return "inc_ptr";
        case CMD_DECREMENT_PTR:
            return "dec_ptr";
        case CMD_INCREMENT_DATA:
            return "inc_data";
        case CMD_DECREMENT_DATA:
            return "dec_data";
        case CMD_OUTPUT_BYTE:
            return "output";
        case CMD_INPUT_BYTE:
            return "input";
        case CMD_OPT_CLEAR:
            return "clearloop";
        case CMD_OPT_SCAN_LEFT:
            return "scan_left";
        case CMD_OPT_SCAN_RIGHT:
            return "scan_right";

        default:
            UNREACHABLE();
    }
}

static void ast_dump(node_t *node, int spaces)
{
    LIST_FOREACH(node->children, c) {
        node_t *n = c->value;
        switch(n->type) {
            case EXPR_OPT:
            case EXPR_PTR:
            case EXPR_DATA:
            case EXPR_IO: {
                ast_print_spaces(spaces);
                printf("%s:%d\n", ast_val_to_str(n->value), n->count);
            } break;

            case EXPR_LOOP: {
                ast_dump(n, spaces + 4);
            } break;

            default:
                UNREACHABLE();
        }
    }
}

void ast_stats(bf_t *prog)
{
    ast_dump(prog->root, 0);
}
