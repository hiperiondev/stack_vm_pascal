/*
 * @ lexical.h
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

#ifndef _LEXICAL_H_
#define _LEXICAL_H_

// Define all token enumeration
typedef enum _token_enum {
    // Book-keeping Token
    ENDFILE,      // 0x00
    ERROR,        // 0x01

    // Reserved Key Word
    KW_ARRAY,     // 0x02
    KW_BEGIN,     // 0x03
    KW_CHAR,      // 0x04
    KW_CONST,     // 0x05
    KW_DO,        // 0x06
    KW_DOWNTO,    // 0x07
    KW_ELSE,      // 0x08
    KW_END,       // 0x09
    KW_FOR,       // 0x0a
    KW_FUNCTION,  // 0x0b
    KW_IF,        // 0x0c
    KW_INTEGER,   // 0x0d
    KW_UINTEGER,  // 0x0e
    KW_OF,        // 0x0f
    KW_PROCEDURE, // 0x10
    KW_READ,      // 0x11
    KW_REPEAT,    // 0x12
    KW_THEN,      // 0x13
    KW_TO,        // 0x14
    KW_UNTIL,     // 0x15
    KW_VAR,       // 0x16
    KW_WRITE,     // 0x17

    // Multi-Character Token
    MC_ID,        // 0x18
    MC_CH,        // 0x19
    MC_UNS,       // 0x1a
    MC_STR,       // 0x1b

    // Special Symbol as Token
    SS_PLUS,      // 0x1c
    SS_MINUS,     // 0x1d
    SS_STAR,      // 0x1e
    SS_OVER,      // 0x1f
    SS_EQU,       // 0x20
    SS_LST,       // 0x21
    SS_LEQ,       // 0x22
    SS_GTT,       // 0x23
    SS_GEQ,       // 0x24
    SS_NEQ,       // 0x25
    SS_COMMA,     // 0x26
    SS_SEMI,      // 0x27
    SS_ASGN,      // 0x28
    SS_LPAR,      // 0x29
    SS_RPAR,      // 0x2a
    SS_LBRA,      // 0x2b
    SS_RBRA,      // 0x2c
    SS_LBBR,      // 0x2d
    SS_RBBR,      // 0x2e
    SS_SQUO,      // 0x2f
    SS_DQUO,      // 0x30
    SS_COLON,     // 0x31
    SS_DOT,       // 0x32
} token_t;

#endif /* _LEXICAL_H_ */
