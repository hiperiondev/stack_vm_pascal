/*
 * @irasm_to_stackvm.c
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
#include <stdbool.h>
#include <stdlib.h>

#include "irassembler.h"
#include "irasm_to_stackvm.h"

void irasm_to_stackvm(asm_result_t *irasm, uint32_t irasm_len, asm_result_t **stackvm_asm, uint32_t *stackvm_asm_len) {
    static asm_result_t *asm_result;
    asm_result = malloc(1);

}
