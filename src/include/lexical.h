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

#ifndef _LEXICAL_H_
#define _LEXICAL_H_

// Define all token enumeration
typedef enum _token_enum {
    // Book-keeping Token
    ENDFILE, // 0x00
    ERROR, // 0x01

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
    KW_OF,        // 0x0e
    KW_PROCEDURE, // 0x0f
    KW_READ,      // 0x10
    KW_REPEAT,    // 0x11
    KW_THEN,      // 0x12
    KW_TO,        // 0x13
    KW_UNTIL,     // 0x14
    KW_VAR,       // 0x15
    KW_WRITE,     // 0x16

    // Multi-Character Token
    MC_ID,        // 0x17
    MC_CH,        // 0x18
    MC_UNS,       // 0x19
    MC_STR,       // 0x1a

    // Special Symbol as Token
    SS_PLUS,     // 0x1b
    SS_MINUS,    // 0x1c
    SS_STAR,     // 0x1d
    SS_OVER,     // 0x1e
    SS_EQU,      // 0x1f
    SS_LST,      // 0x20
    SS_LEQ,      // 0x21
    SS_GTT,      // 0x22
    SS_GEQ,      // 0x23
    SS_NEQ,      // 0x24
    SS_COMMA,    // 0x25
    SS_SEMI,     // 0x26
    SS_ASGN,     // 0x27
    SS_LPAR,     // 0x28
    SS_RPAR,     // 0x29
    SS_LBRA,     // 0x2a
    SS_RBRA,     // 0x2b
    SS_LBBR,     // 0x2c
    SS_RBBR,     // 0x2d
    SS_SQUO,     // 0x2e
    SS_DQUO,     // 0x2f
    SS_COLON,    // 0x30
    SS_DOT,      // 0x31
} token_t;

#endif /* _LEXICAL_H_ */
