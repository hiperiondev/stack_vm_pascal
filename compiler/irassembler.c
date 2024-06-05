/*
 * @irassembler.c
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
#include "irassembler.h"

//#define ENABLE_DEBUG
//#define ENABLE_FULL_DEBUG

#define PIDENT(x)           printf(";%*s", (int)strlen(x), "")
#define ARG_STR(arg, value) strcpy(asm_result->arg.str, value)
#define ARG_NUM(arg, value) asm_result->arg.number = value

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

fn_ir_elements_t *fn_ir_elements;
long int fn_ir_elements_qty;

#ifdef ENABLE_FULL_DEBUG
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
#endif

///////////////////// instructions /////////////////////

static void fn_args(syment_t *symbol, uint32_t ident) {
    if (symbol == NULL)
        return;

    param_t *head = symbol->phead;
    if (head == NULL)
        return;

#ifdef ENABLE_DEBUG
    printf(";%*s[arg]\n", ident, "");
#endif

    while (head != NULL) {
        fn_ir_elements[fn_ir_elements_qty].args = realloc(fn_ir_elements[fn_ir_elements_qty].args, (fn_ir_elements[fn_ir_elements_qty].args_qty + 1) * sizeof(fn_ir_args_t));

        strcpy(fn_ir_elements[fn_ir_elements_qty].args[fn_ir_elements[fn_ir_elements_qty].args_qty].name, head->symbol->name);
        strcpy(fn_ir_elements[fn_ir_elements_qty].args[fn_ir_elements[fn_ir_elements_qty].args_qty].label, head->symbol->label);
        fn_ir_elements[fn_ir_elements_qty].args[fn_ir_elements[fn_ir_elements_qty].args_qty].type = head->symbol->type;
        fn_ir_elements[fn_ir_elements_qty].args[fn_ir_elements[fn_ir_elements_qty].args_qty].category = head->symbol->cate;
        ++fn_ir_elements[fn_ir_elements_qty].args_qty;

#ifdef ENABLE_DEBUG
        printf(";%*s%s %u %u ; %s %s %s\n", ident + 2, "", head->symbol->label, head->symbol->cate == BY_VALUE_OBJ ? 0 : 1, head->symbol->type,
                head->symbol->name, category[head->symbol->cate], value_type[head->symbol->type]);
#endif
        head = head->next;
    }
#ifdef ENABLE_DEBUG
    printf(";%*s[end arg]\n", ident, "");
#endif
}

static void fn_locales(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_DEBUG
    printf(";%*s[locale]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == VARIABLE_OBJ || e->cate == ARRAY_OBJ) {
                fn_ir_elements[fn_ir_elements_qty].locales = realloc(fn_ir_elements[fn_ir_elements_qty].locales,
                        (fn_ir_elements[fn_ir_elements_qty].locales_qty + 1) * sizeof(fn_ir_locales_t));
                strcpy(fn_ir_elements[fn_ir_elements_qty].locales[fn_ir_elements[fn_ir_elements_qty].locales_qty].name, e->name);
                strcpy(fn_ir_elements[fn_ir_elements_qty].locales[fn_ir_elements[fn_ir_elements_qty].locales_qty].label, e->label);
                fn_ir_elements[fn_ir_elements_qty].locales[fn_ir_elements[fn_ir_elements_qty].locales_qty].type = e->type;
                fn_ir_elements[fn_ir_elements_qty].locales[fn_ir_elements[fn_ir_elements_qty].locales_qty].category = e->cate;
                ++fn_ir_elements[fn_ir_elements_qty].locales_qty;

#ifdef ENABLE_DEBUG
                printf(";%*s%s %u %u ; %s %s %s\n", ident + 2, "", e->label, e->cate == ARRAY_OBJ ? 1 : 0, e->type, e->name, category[e->cate],
                        value_type[e->type]);
#endif
            }
        }
    }

#ifdef ENABLE_DEBUG
    printf(";%*s[end locale]\n", ident, "");
#endif
}

static void fn_temps(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_DEBUG
    printf(";%*s[temp]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == TEMP_OBJ) {
                fn_ir_elements[fn_ir_elements_qty].temps = realloc(fn_ir_elements[fn_ir_elements_qty].temps,
                        (fn_ir_elements[fn_ir_elements_qty].temps_qty + 1) * sizeof(fn_ir_temps_t));
                strcpy(fn_ir_elements[fn_ir_elements_qty].temps[fn_ir_elements[fn_ir_elements_qty].temps_qty].name, e->name);
                strcpy(fn_ir_elements[fn_ir_elements_qty].temps[fn_ir_elements[fn_ir_elements_qty].temps_qty].label, e->label);
                fn_ir_elements[fn_ir_elements_qty].temps[fn_ir_elements[fn_ir_elements_qty].temps_qty].type = e->type;
                fn_ir_elements[fn_ir_elements_qty].temps[fn_ir_elements[fn_ir_elements_qty].temps_qty].category = e->cate;
                ++fn_ir_elements[fn_ir_elements_qty].temps_qty;

#ifdef ENABLE_DEBUG
                printf(";%*s%s %u; %s %s\n", ident + 2, "", e->label, e->type, e->name, value_type[e->type]);
#endif
            }
        }
    }

#ifdef ENABLE_DEBUG
    printf(";%*s[end temp]\n", ident, "");
#endif
}

static void fn_strings(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_DEBUG
    printf(";%*s[string]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == STRING_OBJ) {
                fn_ir_elements[fn_ir_elements_qty].strings = realloc(fn_ir_elements[fn_ir_elements_qty].strings,
                        (fn_ir_elements[fn_ir_elements_qty].strings_qty + 1) * sizeof(fn_ir_strings_t));
                strcpy(fn_ir_elements[fn_ir_elements_qty].strings[fn_ir_elements[fn_ir_elements_qty].strings_qty].label, e->label);
                strcpy(fn_ir_elements[fn_ir_elements_qty].strings[fn_ir_elements[fn_ir_elements_qty].strings_qty].value, e->str);
                ++fn_ir_elements[fn_ir_elements_qty].strings_qty;

#ifdef ENABLE_DEBUG
                printf(";%*s%s \"%s\"\n", ident + 2, "", e->label, e->str);
#endif
            }
        }
    }

#ifdef ENABLE_DEBUG
    printf(";%*s[end string]\n", ident, "");
#endif
}

////////////////////////////////////////////////////////

static void asmbl_fn_start_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("name%*s args vars tmps label\n", (int) strlen(instruction->d->name) - 4, "");
    printf("%s %s %04d %04d %04d %s\n", opcode[instruction->op], instruction->d->name, instruction->d->scope->argoff, instruction->d->scope->varoff, instruction->d->scope->tmpoff,
            instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->name);
    ARG_NUM(arg2, instruction->d->scope->argoff);
    ARG_NUM(arg3, instruction->d->scope->varoff);
    ARG_NUM(arg4, instruction->d->scope->tmpoff);
    ARG_STR(arg5, instruction->d->label);

    fn_ir_elements = realloc(fn_ir_elements, (fn_ir_elements_qty + 1) * sizeof(fn_ir_elements_t));
    strcpy(fn_ir_elements[fn_ir_elements_qty].name, instruction->d->name);
    strcpy(fn_ir_elements[fn_ir_elements_qty].label, instruction->d->label);

    fn_ir_elements[fn_ir_elements_qty].args = malloc(sizeof(fn_ir_args_t));
    fn_ir_elements[fn_ir_elements_qty].args_qty = 0;
    fn_args(instruction->d, (int) strlen(opcode[instruction->op]));

    fn_ir_elements[fn_ir_elements_qty].locales = malloc(sizeof(fn_ir_locales_t));
    fn_ir_elements[fn_ir_elements_qty].locales_qty = 0;
    fn_locales(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    fn_ir_elements[fn_ir_elements_qty].temps = malloc(sizeof(fn_ir_temps_t));
    fn_ir_elements[fn_ir_elements_qty].temps_qty = 0;
    fn_temps(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    fn_ir_elements[fn_ir_elements_qty].strings = malloc(sizeof(fn_ir_strings_t));
    fn_ir_elements[fn_ir_elements_qty].strings_qty = 0;
    fn_strings(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    ++fn_ir_elements_qty;

#ifdef ENABLE_DEBUG
    printf("\n");
#endif

#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_fn_end_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("name\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->name);
#endif
    ARG_STR(arg1, instruction->d->name);
    ARG_STR(arg2, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_add_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_sub_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_mul_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_div_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_inc_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_dec_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_neg_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1\n", (int) strlen(instruction->d->label) - 2, "");
    printf("%s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_load_array_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to   arry indx\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_store_var_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1\n", (int) strlen(instruction->d->label) - 2, "");
    printf("%s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_NUM(arg3, instruction->r->type);
    ARG_NUM(arg4, instruction->r->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_store_array_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arry val1 indx\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_equ_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_neq_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_gtt_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_geq_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_lst_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_leq_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
    printf("%s %s %s %s\n", opcode[instruction->op], instruction->d->label, instruction->r->label, instruction->s->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_STR(arg2, instruction->r->label);
    ARG_STR(arg3, instruction->s->label);
    ARG_NUM(arg4, instruction->r->type);
    ARG_NUM(arg5, instruction->r->initval);
    ARG_NUM(arg6, instruction->s->type);
    ARG_NUM(arg7, instruction->s->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_jump_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_push_val_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_NUM(arg2, instruction->d->type);
    ARG_NUM(arg3, instruction->d->initval);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_push_addr_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_pop_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    printf("%s\n", opcode[instruction->op]);
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_call_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("func\n");
    printf("%s %s\n", opcode[instruction->op], instruction->r->name);
#endif
    ARG_STR(arg1, instruction->r->name);
    if (instruction->d != NULL)
        ARG_STR(arg2, instruction->d->label);
    else
        ARG_STR(arg2, "");
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_read_int_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_read_uint_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_read_char_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_string_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_int_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_NUM(arg2, instruction->d->type);
    ARG_NUM(arg3, instruction->d->initval);
    ARG_NUM(arg4, instruction->d->cate);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_uint_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_NUM(arg2, instruction->d->type);
    ARG_NUM(arg3, instruction->d->initval);
    ARG_NUM(arg4, instruction->d->cate);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_char_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
    ARG_NUM(arg2, instruction->d->type);
    ARG_NUM(arg3, instruction->d->initval);
    ARG_NUM(arg4, instruction->d->cate);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_label_op(inst_t *instruction, asm_result_t *asm_result) {
#ifdef ENABLE_DEBUG
    PIDENT(opcode[instruction->op]);
    printf("labl\n");
    printf("%s %s\n", opcode[instruction->op], instruction->d->label);
#endif
    ARG_STR(arg1, instruction->d->label);
#ifdef ENABLE_DEBUG
    printf("\n");
#endif
#ifdef ENABLE_FULL_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

////////////////////////////////////////////////////////

uint32_t gen_irasm(asm_result_t **irasm_result) {
    inst_t *instruction;
    uint32_t irasm_result_len = 0;
    asm_result_t *ir_result = NULL;

    fn_ir_elements = calloc(1, sizeof(fn_ir_elements_t));
    fn_ir_elements_qty = 0;

    for (instruction = xhead; instruction; instruction = instruction->next) {
        *irasm_result = realloc((*irasm_result), (irasm_result_len + 1) * sizeof(asm_result_t));
        (*irasm_result)[irasm_result_len].op = instruction->op;
        ir_result = &((*irasm_result)[irasm_result_len]);

        switch (instruction->op) {
        case ADD_OP:
            asmbl_add_op(instruction, ir_result);
            break;
        case SUB_OP:
            asmbl_sub_op(instruction, ir_result);
            break;
        case MUL_OP:
            asmbl_mul_op(instruction, ir_result);
            break;
        case DIV_OP:
            asmbl_div_op(instruction, ir_result);
            break;
        case INC_OP:
            asmbl_inc_op(instruction, ir_result);
            break;
        case DEC_OP:
            asmbl_dec_op(instruction, ir_result);
            break;
        case NEG_OP:
            asmbl_neg_op(instruction, ir_result);
            break;
        case LOAD_ARRAY_OP:
            asmbl_load_array_op(instruction, ir_result);
            break;
        case STORE_VAR_OP:
            asmbl_store_var_op(instruction, ir_result);
            break;
        case STORE_ARRAY_OP:
            asmbl_store_array_op(instruction, ir_result);
            break;
        case BRANCH_EQU_OP:
            asmbl_branch_equ_op(instruction, ir_result);
            break;
        case BRANCH_NEQ_OP:
            asmbl_branch_neq_op(instruction, ir_result);
            break;
        case BRANCH_GTT_OP:
            asmbl_branch_gtt_op(instruction, ir_result);
            break;
        case BRANCH_GEQ_OP:
            asmbl_branch_geq_op(instruction, ir_result);
            break;
        case BRANCH_LST_OP:
            asmbl_branch_lst_op(instruction, ir_result);
            break;
        case BRANCH_LEQ_OP:
            asmbl_branch_leq_op(instruction, ir_result);
            break;
        case JUMP_OP:
            asmbl_jump_op(instruction, ir_result);
            break;
        case PUSH_VAL_OP:
            asmbl_push_val_op(instruction,ir_result);
            break;
        case PUSH_ADDR_OP:
            asmbl_push_addr_op(instruction, ir_result);
            break;
        case POP_OP:
            asmbl_pop_op(instruction, ir_result);
            break;
        case CALL_OP:
            asmbl_call_op(instruction, ir_result);
            break;
        case FN_START_OP:
            asmbl_fn_start_op(instruction, ir_result);
            break;
        case FN_END_OP:
            asmbl_fn_end_op(instruction, ir_result);
            break;
        case READ_INT_OP:
            asmbl_read_int_op(instruction, ir_result);
            break;
        case READ_UINT_OP:
            asmbl_read_uint_op(instruction, ir_result);
            break;
        case READ_CHAR_OP:
            asmbl_read_char_op(instruction, ir_result);
            break;
        case WRITE_STRING_OP:
            asmbl_write_string_op(instruction, ir_result);
            break;
        case WRITE_INT_OP:
            asmbl_write_int_op(instruction, ir_result);
            break;
        case WRITE_UINT_OP:
            asmbl_write_uint_op(instruction, ir_result);
            break;
        case WRITE_CHAR_OP:
            asmbl_write_char_op(instruction, ir_result);
            break;
        case LABEL_OP:
            asmbl_label_op(instruction, ir_result);
            break;
        default:
            unlikely();
        }

        ++irasm_result_len;
    }

    chkerr("assemble fail and exit.");
    phase = ASSEMBLE;

    printf("\n");

    return irasm_result_len;
}

void print_ir_fn_elements(void) {
    long int fn;

    for (fn = 0; fn < fn_ir_elements_qty; fn++) {
        printf("fn_label %s %s\n", fn_ir_elements[fn].name, fn_ir_elements[fn].label);

        for (long int args = 0; args < fn_ir_elements[fn].args_qty; args++) {
            printf("fn_arg %s %s ", fn_ir_elements[fn].name, fn_ir_elements[fn].args[args].label);
            printf("%s ", category[fn_ir_elements[fn].args[args].category]);
            printf("%s ", value_type[fn_ir_elements[fn].args[args].type]);
            printf("%s\n", fn_ir_elements[fn].args[args].name);
        }

        for (long int locales = 0; locales < fn_ir_elements[fn].locales_qty; locales++) {
            printf("fn_locale %s %s ", fn_ir_elements[fn].name, fn_ir_elements[fn].locales[locales].label);
            printf("%s ", category[fn_ir_elements[fn].locales[locales].category]);
            printf("%s ", category[fn_ir_elements[fn].locales[locales].type]);
            printf("%s\n", fn_ir_elements[fn].locales[locales].name);
        }

        for (long int temps = 0; temps < fn_ir_elements[fn].temps_qty; temps++) {
            printf("fn_temp %s %s ", fn_ir_elements[fn].name, fn_ir_elements[fn].temps[temps].label);
            printf("%s ", category[fn_ir_elements[fn].temps[temps].category]);
            printf("%s ", category[fn_ir_elements[fn].temps[temps].type]);
            printf("%s\n", fn_ir_elements[fn].temps[temps].name);
        }

        for (long int strings = 0; strings < fn_ir_elements[fn].strings_qty; strings++) {
            printf("fn_string %s %s ", fn_ir_elements[fn].name, fn_ir_elements[fn].strings[strings].label);
            printf("\"%s\"\n", fn_ir_elements[fn].strings[strings].value);
        }

        printf("\n");
    }
}

void print_irasm(asm_result_t *irasm_result, uint32_t irasm_result_len) {
    asm_result_t a;
    for (uint32_t line = 0; line < irasm_result_len; ++line) {
        a = irasm_result[line];
        printf("%s ", opcode[a.op]);
        switch (a.op) {
            case ADD_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case SUB_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case MUL_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case DIV_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case INC_OP:
                printf("%s\n", a.arg1.str);
                break;
            case DEC_OP:
                printf("%s\n", a.arg1.str);
                break;
            case NEG_OP:
                printf("%s %s\n", a.arg1.str, a.arg2.str);
                break;
            case LOAD_ARRAY_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case STORE_VAR_OP:
                if (a.arg3.number == LITERAL_TYPE)
                    printf("%s %ld\n", a.arg1.str, a.arg4.number);
                else
                    printf("%s %s\n", a.arg1.str, a.arg2.str);
                break;
            case STORE_ARRAY_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case BRANCH_EQU_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case BRANCH_NEQ_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case BRANCH_GTT_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case BRANCH_GEQ_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case BRANCH_LST_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case BRANCH_LEQ_OP:
                printf("%s ", a.arg1.str);
                if (a.arg4.number == LITERAL_TYPE)
                    printf("%ld ", a.arg5.number);
                else
                    printf("%s ", a.arg2.str);

                if (a.arg6.number == LITERAL_TYPE)
                    printf("%ld \n", a.arg7.number);
                else
                    printf("%s \n", a.arg3.str);
                break;
            case JUMP_OP:
                printf("%s\n", a.arg1.str);
                break;
            case PUSH_VAL_OP:
                if (a.arg2.number == LITERAL_TYPE)
                    printf("%ld\n", a.arg3.number);
                else
                    printf("%s\n", a.arg1.str);
                break;
            case PUSH_ADDR_OP:
                printf("%s\n", a.arg1.str);
                break;
            case POP_OP:
                printf("\n");
                break;
            case CALL_OP:
                printf("%s %s\n", a.arg1.str, a.arg2.str);
                break;
            case FN_START_OP:
                printf("%s %ld %ld %ld %s\n", a.arg1.str, a.arg2.number, a.arg3.number, a.arg4.number, a.arg5.str);
                break;
            case FN_END_OP:
                printf("%s %s\n\n", a.arg1.str, a.arg2.str);
                break;
            case READ_INT_OP:
                printf("%s\n", a.arg1.str);
                break;
            case READ_UINT_OP:
                printf("%s\n", a.arg1.str);
                break;
            case READ_CHAR_OP:
                printf("%s\n", a.arg1.str);
                break;
            case WRITE_STRING_OP:
                if (a.arg2.number == LITERAL_TYPE)
                    printf("%ld\n", a.arg3.number);
                else
                    printf("%s\n", a.arg1.str);
                break;
            case WRITE_INT_OP:
                if (a.arg4.number == NUMBER_OBJ)
                    printf("%ld\n", a.arg3.number);
                else
                    printf("%s\n", a.arg1.str);
                break;
            case WRITE_UINT_OP:
                if (a.arg4.number == NUMBER_OBJ)
                    printf("%ld\n", a.arg3.number);
                else
                    printf("%s\n", a.arg1.str);
                break;
            case WRITE_CHAR_OP:
                if (a.arg4.number == NUMBER_OBJ)
                    printf("%ld\n", a.arg3.number);
                else
                    printf("%s\n", a.arg1.str);
                break;
            case LABEL_OP:
                if (a.arg2.number == LITERAL_TYPE)
                    printf("%ld\n", a.arg3.number);
                else
                    printf("%s\n", a.arg1.str);
                break;
        }

    }
}

void free_irasm(void) {
    // free assembler
    for (long int fn = 0; fn < fn_ir_elements_qty; fn++) {
        free(fn_ir_elements[fn].args);
        free(fn_ir_elements[fn].locales);
        free(fn_ir_elements[fn].temps);
        free(fn_ir_elements[fn].strings);
    }
    free(fn_ir_elements);
}
