#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

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
            UNREACHABLE();
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
            UNREACHABLE();
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
    clock_t begin = clock();
    bf_t *bf_prog = malloc(sizeof(bf_t));
    bf_prog->root = malloc(sizeof(node_t));
    bf_prog->count = 1;

    node_t *current = bf_prog->root;
    current->parent = NULL;
    current->type = EXPR_ROOT;
    current->count = 0;
    current->children = list_init();

    for(int8_t c = io_read_next(); c != EOF; c = io_read_next()) {
        ast_command_t command = ast_char_to_command(c);
        int32_t count = 1;

        if(command == CMD_DECREMENT_DATA || command == CMD_DECREMENT_PTR)
            count = -1;

        // simplify ++/-- and >>/<< instructions into a single instruction and count;
        if(optimisations && ast_command_to_expr(command) != EXPR_LOOP) {
            for(int8_t peek = io_peek(); peek != EOF; peek = io_peek())
            {
                ast_command_t peeked = ast_char_to_command(peek);
                if(ast_command_to_expr(peeked) != ast_command_to_expr(command))
                    break;

                switch(peeked)
                {
                    case CMD_OUTPUT_BYTE:
                    case CMD_INPUT_BYTE:
                    case CMD_INCREMENT_DATA:
                    case CMD_INCREMENT_PTR:
                        count++;
                        break;

                    case CMD_DECREMENT_DATA:
                    case CMD_DECREMENT_PTR:
                        count--;
                        break;

                    default:
                        printf("%d\n", peeked);
                        UNREACHABLE();
                }

                (void)io_read_next();
            }

            // we resolved the +/> and -/< down to nothing
            // we can simply skip creating this node
            // * This continues on the main loop
            if(count == 0)
                continue;

            if(count < 0) {
                if(command == CMD_INCREMENT_DATA)
                    command = CMD_DECREMENT_DATA;

                if(command == CMD_INCREMENT_PTR)
                    command = CMD_DECREMENT_PTR;
            } else {
                if(command == CMD_DECREMENT_DATA)
                    command = CMD_INCREMENT_DATA;

                if(command == CMD_DECREMENT_PTR)
                    command = CMD_INCREMENT_PTR;
            }
        }

        node_t *new;
        // save a little by not allocating loop ends
        // loop ends just change the scope by returning
        // to the parent node
        if(command != CMD_LOOP_END) {
            new = malloc(sizeof(node_t));
            new->count = abs(count);
            new->parent = current;
        }

        switch(command) {
            case CMD_INCREMENT_PTR:
            case CMD_DECREMENT_PTR:
            case CMD_OUTPUT_BYTE:
            case CMD_INPUT_BYTE:
            case CMD_INCREMENT_DATA:
            case CMD_DECREMENT_DATA: {
                new->type = ast_command_to_expr(command);
                new->value = command;
                list_add_end(current->children, new);
                current->count++;
                if(current != bf_prog->root)
                    bf_prog->root->count++;
            } break;
            case CMD_LOOP_START: {
                new->type = EXPR_LOOP;
                new->children = list_init();
                list_add_end(current->children, new);
                current = new;
                // ? loops don't count as instructions in our ast
                // ? maybe they should
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


    bf_prog->generation_time = (double)(clock() - begin) / CLOCKS_PER_SEC;

    return bf_prog;
}

static void ast_printf(int spaces, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    printf("%*s", spaces, "");
    vprintf(format, args);
    putchar('\n');

    va_end(args);
}

static char* ast_cmd_to_str(ast_command_t val)
{
    switch(val) {
        case CMD_INCREMENT_PTR:
            return "CMD_INCREMENT_PTR";
        case CMD_DECREMENT_PTR:
            return "CMD_DECREMENT_PTR";
        case CMD_INCREMENT_DATA:
            return "CMD_INCREMENT_DATA";
        case CMD_DECREMENT_DATA:
            return "CMD_DECREMENT_DATA";
        case CMD_OUTPUT_BYTE:
            return "CMD_OUTPUT_BYTE";
        case CMD_INPUT_BYTE:
            return "CMD_INPUT_BYTE";
        case CMD_OPT_CLEAR:
            return "CMD_OPT_CLEAR";
        case CMD_OPT_SCAN_LEFT:
            return "CMD_OPT_SCAN_LEFT";
        case CMD_OPT_SCAN_RIGHT:
            return "CMD_OPT_SCAN_RIGHT";

        default:
            UNREACHABLE();
    }
}

static void ast_dump_internal(node_t *node, int spaces)
{
    LIST_FOREACH(node->children, c) {
        node_t *n = c->value;
        switch(n->type) {
            case EXPR_OPT:
            case EXPR_PTR:
            case EXPR_DATA:
            case EXPR_IO: {
                ast_printf(spaces, "%-18s : %d", ast_cmd_to_str(n->value), n->count);
            } break;

            case EXPR_LOOP: {
                ast_printf(spaces, "CMD_LOOP_START");
                ast_dump_internal(n, spaces + 4);
                ast_printf(spaces, "CMD_LOOP_END");
            } break;

            default:
                UNREACHABLE();
        }
    }
}
static void ast_dump(node_t *node)
{
    ast_dump_internal(node, 0);
}


void ast_stats(bf_t *prog)
{
    printf(ANSI_BLUE "%-18s : COUNT\n" ANSI_RESET, "INSTRUCTION");
    ast_dump(prog->root);
    printf(ANSI_BLUE "instructions: %d\n" ANSI_RESET, prog->root->count);
    printf(ANSI_BLUE "generation: %fs\n" ANSI_RESET, prog->generation_time);
}
