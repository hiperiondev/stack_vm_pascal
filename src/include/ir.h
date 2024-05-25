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

#ifndef _IR_H_
#define _IR_H_

#include "symtab.h"

extern char *opcode[32];

// Instruction Operator Type
typedef enum _inst_op_enum {
    // Arithmetic
    ADD_OP,  // 0x00 d, e, x
    SUB_OP,  // 0x01 d, e, r
    MUL_OP,  // 0x02 d, e, r
    DIV_OP,  // 0x03 d, e, r
    INC_OP,  // 0x04 d
    DEC_OP,  // 0x05 d
    NEG_OP,  // 0x06 d, r
    // Load and Store
    LOAD_OP, // 0x07 d, r, e
    ASS_OP,  // 0x08 d, r / d, beg
    ASA_OP,  // 0x09 d, r, s
    // Conditional Branch
    EQU_OP,  // 0x0a label, r, s
    NEQ_OP,  // 0x0b label, r s
    GTT_OP,  // 0x0c fordone, d, end / label, r, s
    GEQ_OP,  // 0x0d label, r, s
    LST_OP,  // 0x0e fordone, d, end / lable, r, s
    LEQ_OP,  // 0x0f label, r, s
    // Unconditional Branch
    JMP_OP,  // 0x10 ifdone / loopstart / forstart
    // Stack Management
    PUSH_OP, // 0x11 d
    PADR_OP, // 0x12 d, NULL / d, r
    POP_OP,  // 0x13 NULL
    // Function Management
    CALL_OP, // 0x14 d, e
    ENT_OP,  // 0x15 entry
    FIN_OP,  // 0x16 entry
    // I/O Management
    RDI_OP,  // 0x17 d
    RDC_OP,  // 0x18 d
    WRS_OP,  // 0x19 d
    WRI_OP,  // 0x1a d
    WRC_OP,  // 0x1b d
    // Label Marker
    LAB_OP   // 0x1c ifthen / ifdone / loopstart / loopdone / forstart / fordone
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
