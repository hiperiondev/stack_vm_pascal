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
        EP(NOP_OBJ),   //
        EP(CONST_OBJ), //
        EP(VAR_OBJ),   //
        EP(PROC_OBJ),  //
        EP(FUN_OBJ),   //
        EP(ARRAY_OBJ), //
        EP(BYVAL_OBJ), //
        EP(BYREF_OBJ), //
        EP(TMP_OBJ),   //
        EP(LABEL_OBJ), //
        EP(NUM_OBJ),   //
        EP(STR_OBJ),   //
};

static const char *value_type[] = {
        EP(VOID_TYPE), //
        EP(INT_TYPE),  //
        EP(CHAR_TYPE), //
        EP(STR_TYPE)   //
};

static void print_table(symtab_t *table) {
    symtab_t *t = table;

    printf("        stab(tid=%d): depth=%d, nspace=%s\n", t->tid, t->depth, t->nspace);
    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &t->buckets[i];
        for (e = hair->next; e; e = e->next) {
            printf("        sid=%d, name=%s, cate=%s, type=%s, value=%d, label=%s\n", e->sid, e->name, category[e->cate], value_type[e->type], e->initval, e->label);
        }
    }
    printf("        argoff: %d, varoff: %d, tmpoff: %d\n", table->argoff, table->varoff, table->tmpoff);
}

static void print_syment(syment_t *symbol) {
    if (symbol == NULL)
        return;

    printf("    [symbol entry]\n");
    printf("      [ sid: %d / ", symbol->sid);
    printf("name: %s / ", symbol->name);
    printf("category: %s / ", category[symbol->cate]);
    printf("type: %s / ", value_type[symbol->type]);
    printf("initval: %d / ", symbol->initval);
    printf("arrlen: %d / ", symbol->arrlen);
    printf("str:[ %s ] / ", symbol->str);
    printf("label: %s / ", symbol->label);
    printf("off: %d / ", symbol->off);
    printf("lineno: %d ]\n", symbol->lineno);

    if (symbol->scope != NULL) {
        printf("      [scope]\n");
        print_table(symbol->scope);
        printf("      [end scope]\n");
    }

    printf("    [end symbol entry]\n");
}

static void head(syment_t *symbol) {
    if (symbol == NULL)
        return;

    param_t *head = symbol->phead;
    if (head == NULL)
        return;
    printf("    [head]\n");
    while (head != NULL) {
        print_syment(head->symbol);
        head = head->next;
    }
    printf("    [end head]\n");
}

static void print_args(inst_t *instruction) {
    if (instruction->d != NULL) {
        printf("  [arg d]\n");
        head(instruction->d);
        print_syment(instruction->d);
        printf("  [end arg d]\n");
    }

    if (instruction->r != NULL) {
        printf("  [arg r]\n");
        head(instruction->r);
        print_syment(instruction->r);
        printf("  [end arg r]\n");
    }

    if (instruction->s != NULL) {
        printf("  [arg s]\n");
        head(instruction->s);
        print_syment(instruction->s);
        printf("  [end arg s]\n");
    }
}

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

static void asmbl_ass_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_asa_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_equ_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_neq_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_gtt_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_geq_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_lst_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_leq_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_jmp_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_push_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_padr_op(inst_t *instruction) {
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

static void asmbl_ent_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_fin_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_rdi_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_rdc_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_wrs_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_wri_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_wrc_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

static void asmbl_lab_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
    print_args(instruction);
    printf("\n");
}

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
            case ASS_OP:
                asmbl_ass_op(instruction);
                break;
            case ASA_OP:
                asmbl_asa_op(instruction);
                break;
            case EQU_OP:
                asmbl_equ_op(instruction);
                break;
            case NEQ_OP:
                asmbl_neq_op(instruction);
                break;
            case GTT_OP:
                asmbl_gtt_op(instruction);
                break;
            case GEQ_OP:
                asmbl_geq_op(instruction);
                break;
            case LST_OP:
                asmbl_lst_op(instruction);
                break;
            case LEQ_OP:
                asmbl_leq_op(instruction);
                break;
            case JMP_OP:
                asmbl_jmp_op(instruction);
                break;
            case PUSH_OP:
                asmbl_push_op(instruction);
                break;
            case PADR_OP:
                asmbl_padr_op(instruction);
                break;
            case POP_OP:
                asmbl_pop_op(instruction);
                break;
            case CALL_OP:
                asmbl_call_op(instruction);
                break;
            case ENT_OP:
                asmbl_ent_op(instruction);
                break;
            case FIN_OP:
                asmbl_fin_op(instruction);
                break;
            case RDI_OP:
                asmbl_rdi_op(instruction);
                break;
            case RDC_OP:
                asmbl_rdc_op(instruction);
                break;
            case WRS_OP:
                asmbl_wrs_op(instruction);
                break;
            case WRI_OP:
                asmbl_wri_op(instruction);
                break;
            case WRC_OP:
                asmbl_wrc_op(instruction);
                break;
            case LAB_OP:
                asmbl_lab_op(instruction);
                break;
            default:
                unlikely();
        }

        printf("%s", print_instr);
    }

    chkerr("assemble fail and exit.");
    phase = ASSEMBLE;
}