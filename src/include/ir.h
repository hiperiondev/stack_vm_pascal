/*
 * @ir.h
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

#ifndef _IR_H_
#define _IR_H_

#include "symtab.h"

extern char *opcode[32];

// Instruction Operator Type
typedef enum _inst_op_enum {
    // Arithmetic
    ADD_OP,          // 0x00 d, e, x
    SUB_OP,          // 0x01 d, e, r
    MUL_OP,          // 0x02 d, e, r
    DIV_OP,          // 0x03 d, e, r
    INC_OP,          // 0x04 d
    DEC_OP,          // 0x05 d
    NEG_OP,          // 0x06 d, r

    // Load and Store
    LOAD_ARRAY_OP,   // 0x07 d, r, e
    STORE_VAR_OP,    // 0x08 d, r / d, beg
    STORE_ARRAY_OP,  // 0x09 d, r, s

    // Conditional Branch
    BRANCH_EQU_OP,   // 0x0a label, r, s
    BRANCH_NEQ_OP,   // 0x0b label, r s
    BRANCH_GTT_OP,   // 0x0c fordone, d, end / label, r, s
    BRANCH_GEQ_OP,   // 0x0d label, r, s
    BRANCH_LST_OP,   // 0x0e fordone, d, end / lable, r, s
    BRANCH_LEQ_OP,   // 0x0f label, r, s

    // Unconditional Branch
    JUMP_OP,         // 0x10 ifdone / loopstart / forstart

    // Stack Management
    PUSH_VAL_OP,     // 0x11 d
    PUSH_ADDR_OP,    // 0x12 d, NULL / d, r
    POP_OP,          // 0x13 NULL

    // Function Management
    CALL_OP,         // 0x14 d, e
    FN_START_OP,     // 0x15 entry
    FN_END_OP,       // 0x16 entry

    // I/O Management
    READ_INT_OP,     // 0x17 d
    READ_UINT_OP,    // 0x18 d
    READ_CHAR_OP,    // 0x19 d
    WRITE_STRING_OP, // 0x1a d
    WRITE_INT_OP,    // 0x1b d
    WRITE_UINT_OP,   // 0x1c d
    WRITE_CHAR_OP,   // 0x1d d

    // Label Marker
    LABEL_OP   // 0x1e ifthen / ifdone / loopstart / loopdone / forstart / fordone
} op_t;

// Instruction struct
typedef struct _inst_struct inst_t;

struct _inst_struct {
    int xid;
    op_t op;
    syment_t *d;
    syment_t *r;
    syment_t *s;
    inst_t *prev;
    inst_t *next;
};

// Constructor
#define NEWINST(v) INITMEM(inst_t, v)

// hold instructions
extern inst_t *xhead;
extern inst_t *xtail;

// opcode table
extern char *opcode[32];

// emit an instruction
inst_t* emit1(op_t op, syment_t *d);
inst_t* emit2(op_t op, syment_t *d, syment_t *r);
inst_t* emit3(op_t op, syment_t *d, syment_t *r, syment_t *s);

#endif /* _IR_H_ */
