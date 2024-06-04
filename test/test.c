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
#include <stddef.h>

#include "init.h"
#include "anlysis.h"
#include "assembler.h"
#include "assembler_optimizer.h"
#include "generate.h"
#include "global.h"
#include "scan.h"
#include "parse.h"

void **memtrack;
unsigned long memtrack_qty;

int main(int argc, char *argv[]) {
    pgm_node_t *res = NULL;
    memtrack = malloc(sizeof(void*));
    memtrack_qty = 0;
    asm_result_t *asm_result = calloc(1, sizeof(asm_result_t));
    uint32_t asm_result_len = 0;

    // initial
    init(argc, argv);

    // lexical & syntax
    parse(&res);

    // semantic
    analysis(res);

    // generate IR
    genir(res);

    // generate target code
    asm_result_len = genasm(&asm_result);
    print_asm(asm_result, asm_result_len);
    print_fn_elements();

    //optimize code
    asm_optimize(&asm_result, &asm_result_len);

    // free assembler
    free_asm();
    for (unsigned long n = 0; n < memtrack_qty; n++)
        free(memtrack[n]);
    free(memtrack);
    free(asm_result);

    return 0;
}
