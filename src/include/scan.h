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

#ifndef _SCAN_H_
#define _SCAN_H_

#include "lexical.h"
#include "limits.h"

// token data, store current token string
extern char tokbuf[MAXTOKSIZE + 1];
// token location, line number
extern int toklineno;

// gettok states
typedef enum _state_enum {
    /* 0 */START,
    /* 1 */INSTR,
    /* 2 */INUNS,
    /* 3 */INIDE,
    /* 4 */INLES,
    /* 5 */INCOM,
    /* 6 */INGRE,
    /* 7 */INCHA,
    /* 8 */INCMT,
    /* 9 */DONE
} state_t;

// get next token
token_t gettok(void);

#endif /* _SCAN_H_ */
