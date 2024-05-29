/*
 * @version.h
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

#ifndef _VERSION_H_
#define _VERSION_H_

#define COMPILER_VERSION_MAYOR 0
#define COMPILER_VERSION_MINOR 1
#define COMPILER_VERSION_PATCH 0

///////////////////////////////////////////////////////////////////////////////////
#define STRINGIFY(x) #x
#define COMPILER_VERSION(A,B,C) "v" STRINGIFY(A) "." STRINGIFY(B) "." STRINGIFY(C)
///////////////////////////////////////////////////////////////////////////////////

#endif /* _VERSION_H_ */
