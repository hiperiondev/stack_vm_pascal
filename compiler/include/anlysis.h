/*
 * @anlys.h
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

#ifndef _ANLYSIS_H_
#define _ANLYSIS_H_

#include "global.h"
#include "parse.h"
#include "symtab.h"

void analysis(pgm_node_t *pgm);

#endif /* _ANLYSIS_H_ */
