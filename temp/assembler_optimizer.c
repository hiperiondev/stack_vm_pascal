/*
 * @assembler_optimizer.c
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
#include <stdlib.h>

#include "ir.h"
#include "assembler.h"

//static void correct_call(long int pos, asm_result_t *asm_result, uint32_t asm_result_len) {
//
//}

void asm_optimize(asm_result_t **asm_result, uint32_t *asm_result_len) {
    asm_result_t a;
    for (uint32_t line = 0; line < *asm_result_len; ++line) {
        a = (*asm_result)[line];

        switch (a.op) {
            case ADD_OP:

                break;
            case SUB_OP:

                break;
            case MUL_OP:

                break;
            case DIV_OP:

                break;
            case INC_OP:

                break;
            case DEC_OP:

                break;
            case NEG_OP:

                break;
            case LOAD_ARRAY_OP:

                break;
            case STORE_VAR_OP:

                break;
            case STORE_ARRAY_OP:

                break;
            case BRANCH_EQU_OP:

                break;
            case BRANCH_NEQ_OP:

                break;
            case BRANCH_GTT_OP:

                break;
            case BRANCH_GEQ_OP:

                break;
            case BRANCH_LST_OP:

                break;
            case BRANCH_LEQ_OP:

                break;
            case JUMP_OP:

                break;
            case PUSH_VAL_OP:

                break;
            case PUSH_ADDR_OP:

                break;
            case POP_OP:

                break;
            case CALL_OP:

                break;
            case FN_START_OP:

                break;
            case FN_END_OP:

                break;
            case READ_INT_OP:

                break;
            case READ_UINT_OP:

                break;
            case READ_CHAR_OP:

                break;
            case WRITE_STRING_OP:

                break;
            case WRITE_INT_OP:

                break;
            case WRITE_UINT_OP:

                break;
            case WRITE_CHAR_OP:

                break;
            case LABEL_OP:

                break;
        }

    }
}
