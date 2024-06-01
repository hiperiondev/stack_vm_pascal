/*
 * @assembler_optimizer.h
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

#ifndef ASSEMBLER_OPTIMIZER_H_
#define ASSEMBLER_OPTIMIZER_H_

#include <stdint.h>

#include "assembler.h"

void asm_optimize(asm_result_t **asm_result, uint32_t *asm_result_len);

#endif /* ASSEMBLER_OPTIMIZER_H_ */
