/*
 * @asm.c
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

#include "ir.h"
#include "common.h"
#include "debug.h"
#include "global.h"
#include "symtab.h"
#include "asm.h"

#define PIDENT(x) printf(";%*s", (int)strlen(x), "")

static const char *category[] = {
        "NOP",          //
        "CONST",        //
        "VARIABLE",     //
        "PROCEDURE",    //
        "FUNCTION",     //
        "ARRAY",        //
        "BY_VALUE",     //
        "BY_REFERENCE", //
        "TEMP",         //
        "LABEL",        //
        "NUMBER",       //
        "STRING",       //
};

static const char *value_type[] = {
        "VOID",   // 0
        "INT",    // 1
        "UINT",   // 2
        "CHAR",   // 3
        "STRING", // 4
        "LITERAL" // 5
};

/*
static void print_table(symtab_t *table) {
    printf("          { symbol table id: %d, depth: %d, name space: %s }\n", table->tid, table->depth, table->nspace);

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            printf("          { symbol id: %d, name: %s, category: %s, type: %s, value: %ld, label: %s, offset: %d }\n", e->sid, e->name, category[e->cate], value_type[e->type],
                    e->initval, e->label, e->off);
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
    printf("initval: %ld, ", symbol->initval);
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
*/

///////////////////// instructions /////////////////////

static void fn_args(syment_t *symbol, uint32_t ident) {
    if (symbol == NULL)
        return;

    param_t *head = symbol->phead;
    if (head == NULL)
        return;
    printf("%*s[arg]\n", ident, "");
    while (head != NULL) {
        printf("%*s%s %u %u ; %s %s %s\n", ident + 2, "", head->symbol->label, head->symbol->cate == BY_VALUE_OBJ ? 0 : 1, head->symbol->type, head->symbol->name,
                category[head->symbol->cate], value_type[head->symbol->type]);
        head = head->next;
    }
    printf("%*s[end arg]\n", ident, "");
}

static void fn_locales(symtab_t *table, uint32_t ident) {
    printf("%*s[locale]\n", ident, "");
    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == VARIABLE_OBJ || e->cate == ARRAY_OBJ)
                printf("%*s%s %u %u ; %s %s %s\n", ident + 2, "", e->label, e->cate == ARRAY_OBJ ? 1 : 0, e->type, e->name,
                        category[e->cate], value_type[e->type]);
        }
    }
    printf("%*s[end locale]\n", ident, "");
}

static void fn_temps(symtab_t *table, uint32_t ident) {
    printf("%*s[temp]\n", ident, "");
    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == TEMP_OBJ)
                printf("%*s%s %u %u ; %s %s\n", ident + 2, "", e->label, e->type, (int)e->initval, e->name, value_type[e->type]);
        }
    }
    printf("%*s[end temp]\n", ident, "");
}

static void fn_numbers(symtab_t *table, uint32_t ident) {
    printf("%*s[number]\n", ident, "");
    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == NUMBER_OBJ)
                printf("%*s%s %ld ; LITERAL\n", ident + 2, "", e->label, e->initval);
        }
    }
    printf("%*s[end number]\n", ident, "");
}

static void fn_strings(symtab_t *table, uint32_t ident) {
    printf("%*s[string]\n", ident, "");
    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == STRING_OBJ)
                printf("%*s%s %u %u ; %s %s\n", ident + 2, "", e->label, e->type, (int)e->initval, e->name, value_type[e->type]);
        }
    }
    printf("%*s[end string]\n", ident, "");
}


////////////////////////////////////////////////////////

static void asmbl_fn_start_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("name%*s args vars tmps label\n", (int) strlen(instruction->d->name) - 4, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %04d %04d %04d %s\n", instruction->d->name, instruction->d->scope->argoff, instruction->d->scope->varoff, instruction->d->scope->tmpoff,
            instruction->d->label);

    fn_args(instruction->d, (int) strlen(opcode[instruction->op]) + 1);
    fn_locales(instruction->d->scope, (int) strlen(opcode[instruction->op]) + 1);
    fn_temps(instruction->d->scope, (int) strlen(opcode[instruction->op]) + 1);
    fn_numbers(instruction->d->scope, (int) strlen(opcode[instruction->op]) + 1);
    fn_strings(instruction->d->scope, (int) strlen(opcode[instruction->op]) + 1);

    printf("\n");
    //print_args(instruction);
    //printf("\n");
}

static void asmbl_fn_end_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("name\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->name);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_add_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_sub_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_mul_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_div_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_inc_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_dec_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_neg_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1\n", (int) strlen(instruction->d->label) - 2, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s", instruction->d->label, instruction->r->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_load_array_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to   arry indx\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_store_var_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1\n", (int) strlen(instruction->d->label) - 2, "");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s", instruction->d->label, instruction->r->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_store_array_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arry val1 indx\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_branch_equ_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_branch_neq_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_branch_gtt_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_branch_geq_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_branch_lst_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_branch_leq_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s", instruction->d->label, instruction->r->label, instruction->s->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_jump_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_push_val_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_push_addr_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_pop_op(inst_t *instruction) {
    printf("%s ", opcode[instruction->op]);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_call_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("func\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->r->name);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_read_int_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_read_uint_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_read_char_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_write_string_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_write_int_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_write_uint_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_write_char_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
    printf("\n");
}

static void asmbl_label_op(inst_t *instruction) {
    PIDENT(opcode[instruction->op]);
    printf("labl\n");
    printf("%s ", opcode[instruction->op]);
    printf("%s", instruction->d->label);

    printf("\n");
    //print_args(instruction);
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
            case LOAD_ARRAY_OP:
                asmbl_load_array_op(instruction);
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
