/*
 * @ asm.c
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

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "global.h"
#include "symtab.h"
#include "asm.h"

#define EP(x) [x] = #x

static const char *category[] = {
        "NOP",   //
        "CONST", //
        "VAR",   //
        "PROC",  //
        "FUN",   //
        "ARRAY", //
        "BYVAL", //
        "BYREF", //
        "TMP",   //
        "LABEL", //
        "NUM",   //
        "STR",   //
};

static const char *value_type[] = {
        "VOID", //
        "INT",  //
        "UINT", //
        "CHAR", //
        "STR"   //
};

static void print_table(symtab_t *table) {
    printf("          { symbol table(tid=%d): depth=%d, nspace=%s }\n", table->tid, table->depth, table->nspace);

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            printf("          { symbol id=%d, name=%s, category=%s, type=%s, value=%d, label=%s }\n", e->sid, e->name, category[e->cate], value_type[e->type],
                    e->initval, e->label);
        }
    }
    printf("          { argument offset: %d, variable offset: %d, temp offset: %d }\n", table->argoff, table->varoff, table->tmpoff);
}

static void print_syment(syment_t *symbol) {
    if (symbol == NULL)
        return;

    printf("      [symbol entry]\n");
    printf("        { symbol id: %d, ", symbol->sid);
    printf("name: %s, ", symbol->name);
    printf("category: %s, ", category[symbol->cate]);
    printf("type: %s, ", value_type[symbol->type]);
    printf("initval: %d, ", symbol->initval);
    printf("arrlen: %d, ", symbol->arrlen);
    printf("string: %s, ", strlen(symbol->str) == 0 ? "NULL" : symbol->str);
    printf("label: %s, ", symbol->label);
    printf("offset: %d, ", symbol->off);
    printf("line number: %d }\n", symbol->lineno);

    if (symbol->scope != NULL) {
        printf("        [scope]\n");
        print_table(symbol->scope);
        printf("        [end scope]\n");
    }

    printf("      [end symbol entry]\n");
}

static void head(syment_t *symbol) {
    if (symbol == NULL)
        return;

    param_t *head = symbol->phead;
    if (head == NULL)
        return;
    printf("      [head]\n");
    while (head != NULL) {
        print_syment(head->symbol);
        head = head->next;
    }
    printf("      [end head]\n");
}

static void print_args(inst_t *instruction) {
    printf ("  [args]\n");
    if (instruction->d != NULL) {
        printf("    [arg d]\n");
        head(instruction->d);
        print_syment(instruction->d);
        printf("    [end arg d]\n");
    } else
        printf("    [arg d]\n      { NONE }\n    [end arg d]\n");

    if (instruction->r != NULL) {
        printf("    [arg r]\n");
        head(instruction->r);
        print_syment(instruction->r);
        printf("    [end arg r]\n");
    } else
        printf("    [arg r]\n      { NONE }\n    [end arg r]\n");

    if (instruction->s != NULL) {
        printf("    [arg s]\n");
        head(instruction->s);
        print_syment(instruction->s);
        printf("    [end arg s]\n");
    } else
        printf("    [arg s]\n      { NONE }\n    [end arg s]\n");
    printf ("  [end args]\n");
}

///////////////////// instructions /////////////////////

static void asmbl_add_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_sub_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_mul_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_div_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_inc_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_dec_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_neg_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_load_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_store_var_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_store_array_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_branch_equ_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_branch_neq_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_branch_gtt_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_branch_geq_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_branch_lst_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_branch_leq_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_jump_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_push_val_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_push_addr_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_pop_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_call_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_fn_start_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_fn_end_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_read_int_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_read_uint_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_read_char_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_write_string_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_write_int_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_write_uint_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_write_char_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_label_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

////////////////////////////////////////////////////////

void genasm(void) {
    inst_t *instruction;
    char print_instr[2048];

    for (instruction = xhead; instruction; instruction = instruction->next) {
        memset(print_instr, 0, 2048);
        switch (instruction->op) {
            case ADD_OP:
                asmbl_add_op(instruction);
                break;
            case SUB_OP:
                asmbl_sub_op(instruction);
                break;
            case MUL_OP:
                asmbl_mul_op(instruction);
                break;
            case DIV_OP:
                asmbl_div_op(instruction);
                break;
            case INC_OP:
                asmbl_inc_op(instruction);
                break;
            case DEC_OP:
                asmbl_dec_op(instruction);
                break;
            case NEG_OP:
                asmbl_neg_op(instruction);
                break;
            case LOAD_OP:
                asmbl_load_op(instruction);
                break;
            case STORE_VAR_OP:
                asmbl_store_var_op(instruction);
                break;
            case STORE_ARRAY_OP:
                asmbl_store_array_op(instruction);
                break;
            case BRANCH_EQU_OP:
                asmbl_branch_equ_op(instruction);
                break;
            case BRANCH_NEQ_OP:
                asmbl_branch_neq_op(instruction);
                break;
            case BRANCH_GTT_OP:
                asmbl_branch_gtt_op(instruction);
                break;
            case BRANCH_GEQ_OP:
                asmbl_branch_geq_op(instruction);
                break;
            case BRANCH_LST_OP:
                asmbl_branch_lst_op(instruction);
                break;
            case BRANCH_LEQ_OP:
                asmbl_branch_leq_op(instruction);
                break;
            case JUMP_OP:
                asmbl_jump_op(instruction);
                break;
            case PUSH_VAL_OP:
                asmbl_push_val_op(instruction);
                break;
            case PUSH_ADDR_OP:
                asmbl_push_addr_op(instruction);
                break;
            case POP_OP:
                asmbl_pop_op(instruction);
                break;
            case CALL_OP:
                asmbl_call_op(instruction);
                break;
            case FN_START_OP:
                asmbl_fn_start_op(instruction);
                break;
            case FN_END_OP:
                asmbl_fn_end_op(instruction);
                break;
            case READ_INT_OP:
                asmbl_read_int_op(instruction);
                break;
            case READ_UINT_OP:
                asmbl_read_uint_op(instruction);
                break;
            case READ_CHAR_OP:
                asmbl_read_char_op(instruction);
                break;
            case WRITE_STRING_OP:
                asmbl_write_string_op(instruction);
                break;
            case WRITE_INT_OP:
                asmbl_write_int_op(instruction);
                break;
            case WRITE_UINT_OP:
                asmbl_write_uint_op(instruction);
                break;
            case WRITE_CHAR_OP:
                asmbl_write_char_op(instruction);
                break;
            case LABEL_OP:
                asmbl_label_op(instruction);
                break;
            default:
                unlikely();
        }

        printf("%s", print_instr);
    }

    chkerr("assemble fail and exit.");
    phase = ASSEMBLE;
}
