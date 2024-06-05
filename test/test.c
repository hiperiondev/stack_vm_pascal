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
#include "irassembler.h"
#include "generate.h"
#include "global.h"
#include "scan.h"
#include "parse.h"
#include "irasm_to_stackvm.h"

void **memtrack;
unsigned long memtrack_qty;

int main(int argc, char *argv[]) {
    pgm_node_t *res = NULL;
    memtrack = malloc(sizeof(void*));
    memtrack_qty = 0;
    asm_result_t *irasm = calloc(1, sizeof(asm_result_t));
    uint32_t irasm_len = 0;
    asm_result_t* stackvm_asm = NULL;
    uint32_t stackvm_asm_len = 0;


    // initial
    init(argc, argv);

    // lexical & syntax
    parse(&res);

    // semantic
    analysis(res);

    // generate IR
    genir(res);

    // generate target code
    irasm_len = gen_irasm(&irasm);
    print_irasm(irasm, irasm_len);
    print_ir_fn_elements();

    // generate stackvm asm
    irasm_to_stackvm(irasm, irasm_len, &stackvm_asm, &stackvm_asm_len);

    // free assembler
    free_irasm();
    for (unsigned long n = 0; n < memtrack_qty; n++)
        free(memtrack[n]);
    free(memtrack);
    free(irasm);

    return 0;
}
