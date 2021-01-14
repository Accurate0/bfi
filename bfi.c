#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "ast.h"
#include "util.h"
#include "interpreter.h"

int main(int argc, const char *argv[])
{
    if(argc < 2) {
        die("not enough arguments");
    }

    io_open_file(argv[1]);
    bf_t *prog = ast_init();
    io_close_file();
    interpreter_run(prog->root);
    ast_free(prog);

    return 0;
}
