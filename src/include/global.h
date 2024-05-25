/*
 * @
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

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "debug.h"
#include "limits.h"
#include "lexical.h"

// consts
extern char PL0E_NAME[];
extern char PL0E_VERSION[];
extern char PL0E_INPUT[];
extern char PL0E_ASSEM[];
extern char PL0E_OBJECT[];
extern char PL0E_TARGET[];

// option
extern bool PL0E_OPT_KEEP_NASM_FILE;
extern bool PL0E_OPT_KEEP_OBJECT_FILE;
extern bool PL0E_OPT_SET_TARGET_NAME;

// print control
extern bool echo;
extern bool silent;

// compiler phase
extern phase_t phase;
extern int errnum;

// source code file
extern FILE *source;
extern int lineno;
extern int colmno;
// target assembly file
extern FILE *asmble;

// main entry function name
#define MAINFUNC "_start"

// Lexical
// hold source file line buffer in scan.c
extern char linebuf[MAXLINEBUF];
extern int bufsize;
token_t gettok(void);

#endif /* _GLOBAL_H_ */
