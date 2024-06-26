/*
 * @symtab.c
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

#include "global.h"
#include "debug.h"
#include "limits.h"
#include "parse.h"
#include "util.h"
#include "syntax.h"
#include "symtab.h"

extern void **memtrack;
extern unsigned long memtrack_qty;

// symbol table management
symtab_t *top = NULL;
int depth = 0;
int tidcnt = 0;

syment_t *syments[MAXSYMENT];
int sidcnt = 0;

symtab_t* scope_entry(char *nspace) {
    symtab_t *t;
    NEWSTAB(t);
    t->tid = ++tidcnt;
    t->depth = ++depth;
    strcopy(t->nspace, nspace);
    t->varoff = 1; // reserve function return value

    // Push
    t->outer = top;
    if (top) {
        top->inner = t;
    }
    top = t;

    // trace log
    dbg("push depth=%d tid=%d nspace=%s\n", t->depth, t->tid, t->nspace);
    return t;
}

symtab_t* scope_exit(void) {
    nevernil(top);

    // Pop
    symtab_t *t = top;
    top = t->outer;
    if (top) {
        top->inner = NULL;
    }
    depth--;

    // trace log
    //   1. dump table info
    //   2. dump all table entry
    dbg("pop depth=%d tid=%d nspace=%s\n", t->depth, t->tid, t->nspace);
    int i;
    for (i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &t->buckets[i];
        for (e = hair->next; e; e = e->next) {
            dbg("sid=%d, name=%s\n", e->sid, e->name);
        }
    }
    return t;
}

symtab_t* scope_top(void) {
    nevernil(top);
    return top;
}

// entry management
const int HASHSIZE = 211;
const int HASHSHIFT = 4;

static inline int hash(char *key) {
    if (!key) {
        panic("BAD_HASH_KEY");
    }

    int h, i;
    for (i = h = 0; key[i] != '\0'; i++) {
        h = ((h << HASHSHIFT) + key[i]) % HASHSIZE;
    }

    return h;
}

static syment_t* getsym(symtab_t *stab, char *name) {
    syment_t *hair, *e;
    hair = &stab->buckets[hash(name) % MAXBUCKETS];
    for (e = hair->next; e; e = e->next) {
        if (!strcmp(e->name, name)) {
            return e;
        }
    }
    return NULL;
}

static void putsym(symtab_t *stab, syment_t *e) {
    syment_t *hair = &stab->buckets[hash(e->name) % MAXBUCKETS];
    e->next = hair->next;
    hair->next = e;

    // for debugging
    if (e->sid + 1 >= MAXSYMENT) {
        panic("TOO_MANY_SYMBOL_ENTRY");
    }
    syments[e->sid] = e;

    dbg("tid=%d nspace=%s sym=%s\n", stab->tid, stab->nspace, e->name);
}

static void dumptab(symtab_t *stab) {
    char indent[MAXSTRBUF] = "\0";
    int i;
    for (i = 0; i < stab->depth; ++i) {
        strcat(indent, "  ");
    }

    symtab_t *t = stab;
    msg("%sstab(tid=%d): depth=%d, nspace=%s\n", indent, t->tid, t->depth, t->nspace);

    strcat(indent, "  ");
    for (i = 0; i < MAXBUCKETS; ++i) {
        syment_t *hair, *e;
        hair = &t->buckets[i];
        for (e = hair->next; e; e = e->next) {
            msg("%ssid=%d, name=%s, cate=%d, type=%d, value=%ld, label=%s\n", indent, e->sid, e->name, e->cate, e->type, e->initval, e->label);
        }
    }
    msg("%sargoff: %d, varoff: %d, tmpoff: %d\n", indent, stab->argoff, stab->varoff, stab->tmpoff);
}

syment_t* symget(char *name) {
    nevernil(top);
    return getsym(top, name);
}

syment_t* symget2(symtab_t *stab, char *name) {
    return getsym(stab, name);
}

syment_t* symfind(char *name) {
    nevernil(top);
    syment_t *e;
    symtab_t *t;
    e = NULL;
    for (t = top; t; t = t->outer) {
        if ((e = getsym(t, name)) != NULL) {
            return e;
        }
    }
    return e;
}

void symadd(syment_t *entry) {
    symadd2(top, entry);
}

void symadd2(symtab_t *stab, syment_t *entry) {
    nevernil(stab);
    putsym(stab, entry);
    entry->stab = stab;
}

void stabdump(void) {
    msg("DUMP SYMBOL TABLE:\n");
    symtab_t *t;
    for (t = top; t; t = t->outer) {
        dumptab(t);
    }
    msg("\n");
}

syment_t* syminit(ident_node_t *idp) {
    return syminit2(top, idp, idp->name);
}

syment_t* syminit2(symtab_t *stab, ident_node_t *idp, char *key) {
    syment_t *e;
    NEWENTRY(e);
    e->sid = ++sidcnt;

    strcopy(e->name, key);
    e->initval = idp->value;
    e->arrlen = idp->length;
    e->lineno = idp->line;

    switch (idp->kind) {
        case PROC_IDENT:
            e->cate = PROC_OBJ;
            break;
        case INT_FUN_IDENT:
        case UINT_FUN_IDENT:
        case CHAR_FUN_IDENT:
            e->cate = FUNCTION_OBJ;
            break;
        case INT_CONST_IDENT:
        case UINT_CONST_IDENT:
        case CHAR_CONST_IDENT:
            e->cate = CONSTANT_OBJ;
            break;
        case INT_VAR_IDENT:
        case UINT_VAR_IDENT:
        case CHAR_VAR_IDENT:
            e->cate = VARIABLE_OBJ;
            break;
        case INT_ARRVAR_IDENT:
        case UINT_ARRVAR_IDENT:
        case CHAR_ARRVAR_IDENT:
            e->cate = ARRAY_OBJ;
            break;
        case INT_BYVAL_IDENT:
        case UINT_BYVAL_IDENT:
        case CHAR_BYVAL_IDENT:
            e->cate = BY_VALUE_OBJ;
            break;
        case INT_BYADR_IDENT:
        case UINT_BYADR_IDENT:
        case CHAR_BYADR_IDENT:
            e->cate = BY_REFERENCE_OBJ;
            break;
        default:
            e->cate = NOP_OBJ;
    }

    switch (idp->kind) {
        case INT_FUN_IDENT:
        case INT_CONST_IDENT:
        case INT_VAR_IDENT:
        case INT_ARRVAR_IDENT:
        case INT_BYVAL_IDENT:
        case INT_BYADR_IDENT:
            e->type = INT_TYPE;
            break;
        case UINT_FUN_IDENT:
        case UINT_CONST_IDENT:
        case UINT_VAR_IDENT:
        case UINT_ARRVAR_IDENT:
        case UINT_BYVAL_IDENT:
        case UINT_BYADR_IDENT:
            e->type = UINT_TYPE;
            break;
        case CHAR_FUN_IDENT:
        case CHAR_CONST_IDENT:
        case CHAR_VAR_IDENT:
        case CHAR_ARRVAR_IDENT:
        case CHAR_BYVAL_IDENT:
        case CHAR_BYADR_IDENT:
            e->type = CHAR_TYPE;
            break;
        default:
            e->type = VOID_TYPE;
    }

    switch (e->cate) {
        case NOP_OBJ:
        case CONSTANT_OBJ:
            sprintf(e->label, "CNS%03d", e->sid);
            // no need allocation
            break;
        case VARIABLE_OBJ:
            sprintf(e->label, "VBL%03d", e->sid);
            e->off = stab->varoff;
            stab->varoff++;
            break;
        case PROC_OBJ:
        case FUNCTION_OBJ:
            sprintf(e->label, "FUN%03d", e->sid);
            e->off = stab->varoff;
            stab->varoff++;
            break;
        case BY_VALUE_OBJ:
        case BY_REFERENCE_OBJ:
            sprintf(e->label, "VAL%03d", e->sid);
            e->off = stab->argoff;
            stab->argoff++;
            break;
        case ARRAY_OBJ:
            sprintf(e->label, "ARR%03d", e->sid);
            e->off = stab->varoff;
            stab->varoff += e->arrlen;
            break;
        default:
            unlikely();
    }

    symadd2(stab, e);
    return e;
}

syment_t* symalloc(symtab_t *stab, char *name, cate_t cate, type_t type) {
    syment_t *e;
    NEWENTRY(e);
    strcopy(e->name, name);
    e->sid = ++sidcnt;

    e->cate = cate;
    e->type = type;

    switch (e->cate) {
        case NUMBER_OBJ:
            sprintf(e->label, "LIT%03d", e->sid);
            break;
        case TEMP_OBJ:
            // from now on, we will NEVER alloc local variables so just
            // alloc temporary variables
            sprintf(e->label, "TMP%03d", e->sid);
            e->off = stab->varoff + stab->tmpoff;
            stab->tmpoff++;
            break;
        case LABEL_OBJ:
            sprintf(e->label, "LBL%03d", e->sid);
            break;
        case STRING_OBJ:
            sprintf(e->label, "TMP%03d", e->sid);
            // label/number/string never use bytes
            break;
        default:
            unlikely();
    }

    e->stab = stab;
    putsym(stab, e);
    return e;
}
