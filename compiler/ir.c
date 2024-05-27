/*
 * @ir.c
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

#include "ir.h"
#include "symtab.h"

extern void **memtrack;
extern unsigned long memtrack_qty;

// OPCODE Table
char *opcode[32] = {
        [0] = "ADD",
        [1] = "SUB",
        [2] = "MUL",
        [3] = "DIV",
        [4] = "INC",
        [5] = "DEC",
        [6] = "NEG",
        [7] = "LOAD_ARRAY",
        [8] = "STORE_VAR",
        [9] = "STORE_ARRAY",
        [10] = "BRANCH_EQU",
        [11] = "BRANCH_NEQ",
        [12] = "BRANCH_GTT",
        [13] = "BRANCH_GEQ",
        [14] = "BRANCH_LST",
        [15] = "BRANCH_LEQ",
        [16] = "JUMP",
        [17] = "PUSH_VAL",
        [18] = "PUSH_ADDR",
        [19] = "POP",
        [20] = "CALL",
        [21] = "FN_START",
        [22] = "FN_END",
        [23] = "READ_INT",
        [24] = "READ_UINT",
        [25] = "READ_CHAR",
        [26] = "WRITE_STRING",
        [27] = "WRITE_INT",
        [28] = "WRITE_UINT",
        [29] = "WRITE_CHAR",
        [30] = "LABEL",
};

// instructions
inst_t *xhead;
inst_t *xtail;

// instruction count
int xidcnt = 0;

static inst_t* emit(op_t op) {
    inst_t *t;
    NEWINST(t);
    t->xid = ++xidcnt;
    t->op = op;

    if (xtail) {
        t->prev = xtail;
        xtail->next = t;
        xtail = t;
    } else {
        t->prev = xtail;
        xhead = xtail = t;
    }

    dbg("emit xid=%d op=%d\n", t->xid, op);
    return t;
}

inst_t* emit1(op_t op, syment_t *d) {
    inst_t *x = emit(op);
    x->d = d;
    return x;
}

inst_t* emit2(op_t op, syment_t *d, syment_t *r) {
    inst_t *x = emit(op);
    x->d = d;
    x->r = r;
    return x;
}

inst_t* emit3(op_t op, syment_t *d, syment_t *r, syment_t *s) {
    inst_t *x = emit(op);
    x->d = d;
    x->r = r;
    x->s = s;
    return x;
}
