#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "io.h"
#include "ast.h"
#include "util.h"
#include "interpreter.h"
#include "jit.h"

static void help(const char *name)
{
    printf("%s [ -i | -c | -a ] [ -o | -h ] filename ...\n", name);
}

static void run(bool optimisations, bool jit, bool ast, bool interpret)
{
    bf_t *prog = ast_init(optimisations);

    if(optimisations)
        ast_optimise(prog->root);

    if(ast)
        ast_stats(prog);

    if(interpret)
        interpreter_run(prog->root);

    if(jit)
        jit_run(prog->root);

    ast_free(prog);
}

int main(int argc, char *const argv[])
{
    bool ast = false;
    bool jit = false;
    bool interpret = false;
    bool optimisations = true;
    int c;
    while((c = getopt(argc, argv, "iohja")) != -1) {
        switch(c) {
            case 'j':
                jit = true;
                break;
            case 'i':
                interpret = true;
                break;
            case 'a':
                ast = true;
                break;
            case 'o':
                optimisations = false;
                break;
            case 'h':
                help(argv[0]);
                break;
        }
    }

    if(!(interpret || jit || ast)) {
        help(argv[0]);
        die("must pick at least one execution type");
    }

    int remainder = argc - optind;
    bool read_stdin = (remainder == 1 && !strcmp(argv[optind], "-"))
                   || (remainder == 0);

    if(read_stdin) {
        io_set_file(stdin);

        run(optimisations, jit, ast, interpret);

        return EXIT_SUCCESS;
    }

    for (int i = optind; i < argc; i++) {
        io_open_file(argv[i]);

        run(optimisations, jit, ast, interpret);

        io_close_file();
    }

    return EXIT_SUCCESS;
}
