#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

#include "jit.h"
#include "ast.h"
#include "util.h"
#include "str.h"

typedef void (*compiled_function)();
static int32_t BF_DATA[30000] = {0,};
static int32_t *BF_PTR = BF_DATA;
static int32_t JIT_INDEX = 0;

static uint32_t jit_generate_code(node_t *node, str_t *str, uint32_t size)
{
    (void)str;
    LIST_FOREACH(node->children, c) {
        node_t *node = c->value;

        switch(node->type) {
            case EXPR_PTR:
            case EXPR_DATA:
            case EXPR_IO:
            case EXPR_OPT: {
                switch(node->value) {
                    case CMD_INCREMENT_PTR:
                        break;

                    case CMD_DECREMENT_PTR:
                        break;

                    case CMD_INCREMENT_DATA:
                        break;

                    case CMD_DECREMENT_DATA:
                        break;

                    case CMD_OUTPUT_BYTE:
                        break;

                    case CMD_INPUT_BYTE:
                        break;

                    case CMD_OPT_CLEAR:
                        break;

                    default:
                        UNREACHABLE();
                }
            } break;

            case EXPR_LOOP: {
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

    return size;
}

typedef void (*jit_function)();

void jit_run(node_t *node)
{
    (void)JIT_INDEX;
    (void)BF_PTR;
    // pretend we average 6 bytes per AST node
    // this will be increase as needed
    // this only needs to be healthy initial value
    str_t *str = str_init(node->count * 6);
    // GENERATE THE CODE FIRST
    size_t size = jit_generate_code(node, str, 0);
    // ALLOCATE MEMORY
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // EXECUTE MEMORY
    mprotect(ptr, size, PROT_READ | PROT_EXEC);
    // ((compiled_function)ptr)();
    // FREE
    str_free(str);
    munmap(ptr, size);
}
