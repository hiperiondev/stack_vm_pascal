/*
 * @assembler.c
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
#include "assembler.h"

//#define ENABLE_COMMENTS
//#define ENABLE_DEBUG

#define PIDENT(x) printf(";%*s", (int)strlen(x), "")

#if defined(ENABLE_COMMENTS) || defined(ENABLE_DEBUG)
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
#endif

fn_elements_t *fn_elements;
long int fn_elements_qty;

#ifdef ENABLE_DEBUG
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

#ifdef ENABLE_COMMENTS
    printf(";%*s[arg]\n", ident, "");
#endif

    while (head != NULL) {
        fn_elements[fn_elements_qty].args = realloc(fn_elements[fn_elements_qty].args, (fn_elements[fn_elements_qty].args_qty + 1) * sizeof(fn_args_t));

        strcpy(fn_elements[fn_elements_qty].args[fn_elements[fn_elements_qty].args_qty].name, head->symbol->name);
        strcpy(fn_elements[fn_elements_qty].args[fn_elements[fn_elements_qty].args_qty].label, head->symbol->label);
        fn_elements[fn_elements_qty].args[fn_elements[fn_elements_qty].args_qty].type = head->symbol->type;
        fn_elements[fn_elements_qty].args[fn_elements[fn_elements_qty].args_qty].category = head->symbol->cate == BY_VALUE_OBJ ? 0 : 1;
        ++fn_elements[fn_elements_qty].args_qty;

#ifdef ENABLE_COMMENTS
        printf(";%*s%s %u %u ; %s %s %s\n", ident + 2, "", head->symbol->label, head->symbol->cate == BY_VALUE_OBJ ? 0 : 1, head->symbol->type,
                head->symbol->name, category[head->symbol->cate], value_type[head->symbol->type]);
#endif
        head = head->next;
    }
#ifdef ENABLE_COMMENTS
    printf(";%*s[end arg]\n", ident, "");
#endif
}

static void fn_locales(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_COMMENTS
    printf(";%*s[locale]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == VARIABLE_OBJ || e->cate == ARRAY_OBJ) {
                fn_elements[fn_elements_qty].locales = realloc(fn_elements[fn_elements_qty].locales,
                        (fn_elements[fn_elements_qty].locales_qty + 1) * sizeof(fn_locales_t));
                strcpy(fn_elements[fn_elements_qty].locales[fn_elements[fn_elements_qty].locales_qty].name, e->name);
                strcpy(fn_elements[fn_elements_qty].locales[fn_elements[fn_elements_qty].locales_qty].label, e->label);
                fn_elements[fn_elements_qty].locales[fn_elements[fn_elements_qty].locales_qty].type = e->type;
                fn_elements[fn_elements_qty].locales[fn_elements[fn_elements_qty].locales_qty].category = e->cate == ARRAY_OBJ ? 1 : 0;
                ++fn_elements[fn_elements_qty].locales_qty;

#ifdef ENABLE_COMMENTS
                printf(";%*s%s %u %u ; %s %s %s\n", ident + 2, "", e->label, e->cate == ARRAY_OBJ ? 1 : 0, e->type, e->name, category[e->cate],
                        value_type[e->type]);
#endif
            }
        }
    }

#ifdef ENABLE_COMMENTS
    printf(";%*s[end locale]\n", ident, "");
#endif
}

static void fn_temps(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_COMMENTS
    printf(";%*s[temp]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == TEMP_OBJ) {
                fn_elements[fn_elements_qty].temps = realloc(fn_elements[fn_elements_qty].temps,
                        (fn_elements[fn_elements_qty].temps_qty + 1) * sizeof(fn_temps_t));
                strcpy(fn_elements[fn_elements_qty].temps[fn_elements[fn_elements_qty].temps_qty].name, e->name);
                strcpy(fn_elements[fn_elements_qty].temps[fn_elements[fn_elements_qty].temps_qty].label, e->label);
                fn_elements[fn_elements_qty].temps[fn_elements[fn_elements_qty].temps_qty].type = e->type;
                fn_elements[fn_elements_qty].temps[fn_elements[fn_elements_qty].temps_qty].category = e->cate == ARRAY_OBJ ? 1 : 0;
                ++fn_elements[fn_elements_qty].temps_qty;

#ifdef ENABLE_COMMENTS
                printf(";%*s%s %u; %s %s\n", ident + 2, "", e->label, e->type, e->name, value_type[e->type]);
#endif
            }
        }
    }

#ifdef ENABLE_COMMENTS
    printf(";%*s[end temp]\n", ident, "");
#endif
}

static void fn_literals(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_COMMENTS
    printf(";%*s[literal]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == NUMBER_OBJ) {
                fn_elements[fn_elements_qty].literals = realloc(fn_elements[fn_elements_qty].literals,
                        (fn_elements[fn_elements_qty].literals_qty + 1) * sizeof(fn_literals_t));
                strcpy(fn_elements[fn_elements_qty].literals[fn_elements[fn_elements_qty].literals_qty].label, e->label);
                fn_elements[fn_elements_qty].literals[fn_elements[fn_elements_qty].literals_qty].value = e->initval;
                ++fn_elements[fn_elements_qty].literals_qty;

#ifdef ENABLE_COMMENTS
                printf(";%*s%s %ld\n", ident + 2, "", e->label, e->initval);
#endif
            }
        }
    }

#ifdef ENABLE_COMMENTS
    printf(";%*s[end literal]\n", ident, "");
#endif
}

static void fn_strings(symtab_t *table, uint32_t ident) {
#ifdef ENABLE_COMMENTS
    printf(";%*s[string]\n", ident, "");
#endif

    for (int i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &table->buckets[i];
        for (e = hair->next; e; e = e->next) {
            if (e->cate == STRING_OBJ) {
                fn_elements[fn_elements_qty].strings = realloc(fn_elements[fn_elements_qty].strings,
                        (fn_elements[fn_elements_qty].strings_qty + 1) * sizeof(fn_strings_t));
                strcpy(fn_elements[fn_elements_qty].strings[fn_elements[fn_elements_qty].strings_qty].label, e->label);
                strcpy(fn_elements[fn_elements_qty].strings[fn_elements[fn_elements_qty].strings_qty].value, e->str);
                ++fn_elements[fn_elements_qty].strings_qty;

#ifdef ENABLE_COMMENTS
                printf(";%*s%s \"%s\"\n", ident + 2, "", e->label, e->str);
#endif
            }
        }
    }

#ifdef ENABLE_COMMENTS
    printf(";%*s[end string]\n", ident, "");
#endif
}

////////////////////////////////////////////////////////

static void asmbl_fn_start_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("name%*s args vars tmps label\n", (int) strlen(instruction->d->name) - 4, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %04d %04d %04d %s\n", instruction->d->name, instruction->d->scope->argoff, instruction->d->scope->varoff, instruction->d->scope->tmpoff,
            instruction->d->label);

    fn_elements = realloc(fn_elements, (fn_elements_qty + 1) * sizeof(fn_elements_t));
    strcpy(fn_elements[fn_elements_qty].name, instruction->d->name);

    fn_elements[fn_elements_qty].args = malloc(sizeof(fn_args_t));
    fn_elements[fn_elements_qty].args_qty = 0;
    fn_args(instruction->d, (int) strlen(opcode[instruction->op]));

    fn_elements[fn_elements_qty].locales = malloc(sizeof(fn_locales_t));
    fn_elements[fn_elements_qty].locales_qty = 0;
    fn_locales(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    fn_elements[fn_elements_qty].literals = malloc(sizeof(fn_literals_t));
    fn_elements[fn_elements_qty].literals_qty = 0;
    fn_literals(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    fn_elements[fn_elements_qty].temps = malloc(sizeof(fn_temps_t));
    fn_elements[fn_elements_qty].temps_qty = 0;
    fn_temps(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    fn_elements[fn_elements_qty].strings = malloc(sizeof(fn_strings_t));
    fn_elements[fn_elements_qty].strings_qty = 0;
    fn_strings(instruction->d->scope, (int) strlen(opcode[instruction->op]));

    ++fn_elements_qty;

#ifdef ENABLE_COMMENTS
    printf("\n");
#endif

#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_fn_end_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("name\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->name);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_add_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_sub_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_mul_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_div_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1 %*sarg2\n", (int) strlen(instruction->d->label) - 2, "", (int) strlen(instruction->d->label) - 4, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_inc_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_dec_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_neg_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1\n", (int) strlen(instruction->d->label) - 2, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s\n", instruction->d->label, instruction->r->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_load_array_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to   arry indx\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_store_var_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("to%*s arg1\n", (int) strlen(instruction->d->label) - 2, "");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s\n", instruction->d->label, instruction->r->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_store_array_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arry val1 indx\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_equ_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_neq_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_gtt_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_geq_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_lst_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_branch_leq_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl arg1 arg2\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s %s %s\n", instruction->d->label, instruction->r->label, instruction->s->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_jump_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_push_val_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_push_addr_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_pop_op(inst_t *instruction) {
    printf("%s\n", opcode[instruction->op]);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_call_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("func\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->r->name);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_read_int_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_read_uint_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_read_char_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_string_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_int_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_uint_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_write_char_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("arg1\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

static void asmbl_label_op(inst_t *instruction) {
#ifdef ENABLE_COMMENTS
    PIDENT(opcode[instruction->op]);
    printf("labl\n");
#endif
    printf("%s ", opcode[instruction->op]);
    printf("%s\n", instruction->d->label);
#ifdef ENABLE_COMMENTS
    printf("\n");
#endif
#ifdef ENABLE_DEBUG
    print_args(instruction);
    printf("\n");
#endif
}

////////////////////////////////////////////////////////

void genasm(void) {
    inst_t *instruction;
    char print_instr[2048];
    fn_elements = calloc(1, sizeof(fn_elements_t));
    fn_elements_qty = 0;

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

    printf("\n");
}

void print_fn_elements(void) {
    long int fn;

    for (fn = 0; fn < fn_elements_qty; fn++) {
        for (long int args = 0; args < fn_elements[fn].args_qty; args++) {
            printf("fn_arg %s %s ", fn_elements[fn].name, fn_elements[fn].args[args].label);
            printf("%d ", fn_elements[fn].args[args].category);
            printf("%d ", fn_elements[fn].args[args].type);
            printf("%s\n", fn_elements[fn].args[args].name);
        }

        for (long int locales = 0; locales < fn_elements[fn].locales_qty; locales++) {
            printf("fn_locale %s %s ", fn_elements[fn].name, fn_elements[fn].locales[locales].label);
            printf("%d ", fn_elements[fn].locales[locales].category);
            printf("%d ", fn_elements[fn].locales[locales].type);
            printf("%s\n", fn_elements[fn].locales[locales].name);
        }

        for (long int temps = 0; temps < fn_elements[fn].temps_qty; temps++) {
            printf("fn_temp %s %s ", fn_elements[fn].name, fn_elements[fn].temps[temps].label);
            printf("%d ", fn_elements[fn].temps[temps].category);
            printf("%d ", fn_elements[fn].temps[temps].type);
            printf("%s\n", fn_elements[fn].temps[temps].name);
        }

        for (long int literals = 0; literals < fn_elements[fn].literals_qty; literals++) {
            printf("fn_literal %s %s ", fn_elements[fn].name, fn_elements[fn].literals[literals].label);
            printf("%ld\n", fn_elements[fn].literals[literals].value);
        }

        for (long int strings = 0; strings < fn_elements[fn].strings_qty; strings++) {
            printf("fn_string %s %s ", fn_elements[fn].name, fn_elements[fn].strings[strings].label);
            printf("\"%s\"\n", fn_elements[fn].strings[strings].value);
        }

        printf("\n");
    }
}

void free_asm(void) {
    // free assembler
    for (long int fn = 0; fn < fn_elements_qty; fn++) {
        free(fn_elements[fn].args);
        free(fn_elements[fn].literals);
        free(fn_elements[fn].locales);
        free(fn_elements[fn].temps);
        free(fn_elements[fn].strings);
    }
    free(fn_elements);
}