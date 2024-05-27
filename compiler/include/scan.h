/*
 * @scan.h
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
    START, // 0x00
    INSTR, // 0x01
    INUNS, // 0x02
    INIDE, // 0x03
    INLES, // 0x04
    INCOM, // 0x05
    INGRE, // 0x06
    INCHA, // 0x07
    INCMT, // 0x08
    DONE   // 0x09
} state_t;

// get next token
token_t gettok(void);

#endif /* _SCAN_H_ */
