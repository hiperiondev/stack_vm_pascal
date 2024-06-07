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

#include "ir.h"
#include "stackvm_opcodes.h"
#include "irassembler.h"
#include "irasm_to_stackvm.h"

static void correct_consecutive_operations(asm_result_t *irasm, uint32_t irasm_len, asm_result_t **stackvm_asm, uint32_t *stackvm_asm_len) {
    uint32_t line;

    for (line = 0; line > irasm_len; line++) {

    }

}

void irasm_to_stackvm(asm_result_t *irasm, uint32_t irasm_len, asm_result_t **stackvm_asm, uint32_t *stackvm_asm_len) {
    correct_consecutive_operations(irasm, irasm_len, stackvm_asm, stackvm_asm_len);

}
