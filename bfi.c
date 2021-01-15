#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "io.h"
#include "ast.h"
#include "util.h"
#include "interpreter.h"
#include "jit.h"

static void help(const char *name)
{
    printf("%s [ -i | -c | -a | -o | -h ] filename ...\n", name);
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

    if(!(argc - optind)) {
        help(argv[0]);
        die("not enough arguments");
    }

    if(!(interpret || jit || ast)) {
        help(argv[0]);
        die("must pick at least one execution type");
    }

    for (int i = optind; i < argc; i++) {
        io_open_file(argv[i]);
        bf_t *prog = ast_init(optimisations);
        io_close_file();

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

    return 0;
}
