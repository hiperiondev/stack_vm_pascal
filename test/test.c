/*
 * @main.c
 *
 * @brief Pascal for Stack VM
 * @details
 * This is based on other projects:
 *   Compiler for PL/0 plus language: https://github.com/Jeanhwea/Compiler
 *   Others (see individual files)
 *
 *   please contact their authors for more information.
 *
 * @author Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com)
 * @date 2024
 * @copyright MIT License
 * @see https://github.com/hiperiondev/stack_vm_pascal
 */

#include <stdint.h>

#include "init.h"
#include "anlys.h"
#include "asm.h"
#include "gen.h"
#include "global.h"
#include "scan.h"
#include "parse.h"

void **memtrack;
unsigned long memtrack_qty;

int main(int argc, char *argv[]) {
    pgm_node_t *res = NULL;
    memtrack = malloc(sizeof(void*));
    memtrack_qty = 0;

    // initial
    init(argc, argv);

    // lexical & syntax
    parse(&res);

    // semantic
    analysis(res);

    // generate IR
    genir(res);

    // generate target code
    genasm();

    for (unsigned long n = 0; n < memtrack_qty; n++)
        free(memtrack[n]);
    free(memtrack);

    return 0;
}
