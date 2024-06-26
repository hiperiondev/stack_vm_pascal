/*
 * @generate.c
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

#include "generate.h"
#include "limits.h"
#include "util.h"
#include "debug.h"
#include "parse.h"
#include "syntax.h"
#include "global.h"
#include "symtab.h"
#include "ir.h"

static void gen_pgm(pgm_node_t *node);
static void gen_pf_dec_list(pf_dec_list_node_t *node);
static void gen_proc_decf(proc_dec_node_t *node);
static void gen_fun_decf(fun_dec_node_t *node);
static void gen_comp_stmt(comp_stmt_node_t *node);
static void gen_stmt(stmt_node_t *node);
static void gen_assign_stmt(assign_stmt_node_t *node);
static void gen_if_stmt(if_stmt_node_t *node);
static void gen_repe_stmt(repe_stmt_node_t *node);
static void gen_for_stmt(for_stmt_node_t *node);
static void gen_pcall_stmt(pcall_stmt_node_t *node);
static void gen_read_stmt(read_stmt_node_t *node);
static void gen_write_stmt(write_stmt_node_t *node);
static syment_t* gen_expr(expr_node_t *node);
static syment_t* gen_term(term_node_t *node);
static syment_t* gen_factor(factor_node_t *node);
static syment_t* gen_fcall_stmt(fcall_stmt_node_t *node);
static void gen_cond(cond_node_t *node, syment_t *dest);
static void gen_arg_list(arg_list_node_t *node);

static void gen_pgm(pgm_node_t *node) {
    block_node_t *b = node->bp;
    gen_pf_dec_list(b->pfdlp);

    // main function
    syment_t *entry = node->entry->symbol;
    emit1(FN_START_OP, entry);
    gen_comp_stmt(b->csp);
    emit1(FN_END_OP, entry);
}

static void gen_pf_dec_list(pf_dec_list_node_t *node) {
    pf_dec_list_node_t *t;
    for (t = node; t; t = t->next) {
        switch (t->kind) {
            case PROC_PFDEC:
                gen_proc_decf(t->pdp);
                break;
            case FUN_PFDEC:
                gen_fun_decf(t->fdp);
                break;
            default:
                unlikely();
        }
    }
}

static void gen_proc_decf(proc_dec_node_t *node) {
    proc_dec_node_t *t;
    for (t = node; t; t = t->next) {
        block_node_t *b = t->pdp->bp;
        gen_pf_dec_list(b->pfdlp);

        emit1(FN_START_OP, t->pdp->php->idp->symbol);
        gen_comp_stmt(b->csp);
        emit1(FN_END_OP, t->pdp->php->idp->symbol);
    }
}

static void gen_fun_decf(fun_dec_node_t *node) {
    fun_dec_node_t *t;
    for (t = node; t; t = t->next) {
        block_node_t *b = t->fdp->bp;

        gen_pf_dec_list(b->pfdlp);
        emit1(FN_START_OP, t->fdp->fhp->idp->symbol);
        gen_comp_stmt(b->csp);
        emit1(FN_END_OP, t->fdp->fhp->idp->symbol);
    }
}

static void gen_comp_stmt(comp_stmt_node_t *node) {
    comp_stmt_node_t *t;
    for (t = node; t; t = t->next) {
        gen_stmt(t->sp);
    }
}

static void gen_stmt(stmt_node_t *node) {
    switch (node->kind) {
        case ASSGIN_STMT:
            gen_assign_stmt(node->asp);
            break;
        case IF_STMT:
            gen_if_stmt(node->ifp);
            break;
        case REPEAT_STMT:
            gen_repe_stmt(node->rpp);
            break;
        case FOR_STMT:
            gen_for_stmt(node->frp);
            break;
        case PCALL_STMT:
            gen_pcall_stmt(node->pcp);
            break;
        case COMP_STMT:
            gen_comp_stmt(node->cpp);
            break;
        case READ_STMT:
            gen_read_stmt(node->rdp);
            break;
        case WRITE_STMT:
            gen_write_stmt(node->wtp);
            break;
        case NULL_STMT:
            break;
        default:
            unlikely();
    }
}

static void gen_assign_stmt(assign_stmt_node_t *node) {
    syment_t *r, *s, *d;
    d = node->idp->symbol;
    switch (node->kind) {
        case NORM_ASSGIN:
            r = gen_expr(node->rep);
            emit2(STORE_VAR_OP, d, r);
            break;
        case FUN_ASSGIN:
            r = gen_expr(node->rep);
            emit2(STORE_VAR_OP, d, r);
            break;
        case ARRAY_ASSGIN:
            s = gen_expr(node->lep);
            r = gen_expr(node->rep);
            emit3(STORE_ARRAY_OP, d, r, s);
            break;
        default:
            unlikely();
    }
}

static void gen_if_stmt(if_stmt_node_t *node) {
    syment_t *ifthen, *ifdone;
    ifthen = symalloc(node->stab, "@ifthen", LABEL_OBJ, VOID_TYPE);
    ifdone = symalloc(node->stab, "@ifdone", LABEL_OBJ, VOID_TYPE);

    gen_cond(node->cp, ifthen);
    if (node->ep) {
        gen_stmt(node->ep);
    }
    emit1(JUMP_OP, ifdone);
    emit1(LABEL_OP, ifthen);
    gen_stmt(node->tp);
    emit1(LABEL_OP, ifdone);
}

static void gen_repe_stmt(repe_stmt_node_t *node) {
    syment_t *loopstart, *loopdone;
    loopstart = symalloc(node->stab, "@loopstart", LABEL_OBJ, VOID_TYPE);
    loopdone = symalloc(node->stab, "@loopdone", LABEL_OBJ, VOID_TYPE);

    emit1(LABEL_OP, loopstart);
    gen_stmt(node->sp);
    gen_cond(node->cp, loopdone);
    emit1(JUMP_OP, loopstart);
    emit1(LABEL_OP, loopdone);
}

static void gen_for_stmt(for_stmt_node_t *node) {
    syment_t *beg, *end;
    beg = gen_expr(node->lep);
    end = gen_expr(node->rep);

    syment_t *forstart, *fordone;
    forstart = symalloc(node->stab, "@forstart", LABEL_OBJ, VOID_TYPE);
    fordone = symalloc(node->stab, "@fordone", LABEL_OBJ, VOID_TYPE);

    syment_t *d;
    d = node->idp->symbol;
    emit2(STORE_VAR_OP, d, beg);
    emit1(LABEL_OP, forstart);
    switch (node->kind) {
        case TO_FOR:
            emit3(BRANCH_GTT_OP, fordone, d, end);
            gen_stmt(node->sp);
            emit1(INC_OP, d);
            emit1(JUMP_OP, forstart);
            emit1(LABEL_OP, fordone);
            emit1(DEC_OP, d);
            break;
        case DOWNTO_FOR:
            emit3(BRANCH_LST_OP, fordone, d, end);
            gen_stmt(node->sp);
            emit1(DEC_OP, d);
            emit1(JUMP_OP, forstart);
            emit1(LABEL_OP, fordone);
            emit1(INC_OP, d);
            break;
        default:
            unlikely();
    }
}

static void gen_pcall_stmt(pcall_stmt_node_t *node) {
    gen_arg_list(node->alp);
    emit2(CALL_OP, NULL, node->idp->symbol);
    arg_list_node_t *t;
    for (t = node->alp; t; t = t->next) {
        emit1(POP_OP, NULL);
    }
}

static void gen_read_stmt(read_stmt_node_t *node) {
    read_stmt_node_t *t;
    syment_t *d = NULL;
    for (t = node; t; t = t->next) {
        d = t->idp->symbol;
        switch (d->type) {
            case CHAR_TYPE:
                emit1(READ_CHAR_OP, d);
                break;
            case INT_TYPE:
                emit1(READ_INT_OP, d);
                break;
            case UINT_TYPE:
                emit1(READ_UINT_OP, d);
                break;
            default:
        }
    }
}

static void gen_write_stmt(write_stmt_node_t *node) {
    syment_t *d = NULL;
    switch (node->type) {
        case STR_WRITE:
            d = symalloc(node->stab, "@write/str", STRING_OBJ, STRING_TYPE);
            strcopy(d->str, node->sp);
            emit1(WRITE_STRING_OP, d);
            break;
        case ID_WRITE:
            d = gen_expr(node->ep);
            switch (d->type) {
                case CHAR_TYPE:
                    emit1(WRITE_CHAR_OP, d);
                    break;
                case INT_TYPE:
                    emit1(WRITE_INT_OP, d);
                    break;
                case UINT_TYPE:
                    emit1(WRITE_UINT_OP, d);
                    break;
                case LITERAL_TYPE:
                    emit1(WRITE_INT_OP, d);
                    break;
                default:
                    unlikely();
            }
            break;
        case STRID_WRITE:
            d = symalloc(node->stab, "@write/str", STRING_OBJ, STRING_TYPE);
            strcopy(d->str, node->sp);
            emit1(WRITE_STRING_OP, d);
            d = gen_expr(node->ep);
            switch (d->type) {
                case CHAR_TYPE:
                    emit1(WRITE_CHAR_OP, d);
                    break;
                case INT_TYPE:
                    emit1(WRITE_INT_OP, d);
                    break;
                case UINT_TYPE:
                    emit1(WRITE_UINT_OP, d);
                    break;
                default:
                    unlikely();
            }
            break;
        default:
            unlikely();
    }
}

static syment_t* gen_expr(expr_node_t *node) {
    expr_node_t *t;
    syment_t *d, *r, *e;
    d = r = e = NULL;
    for (t = node; t; t = t->next) {
        r = gen_term(t->tp);

        if (!d) {
            switch (t->kind) {
                case NEG_ADDOP:
                    if (r->type == LITERAL_TYPE) {
                        d = symalloc(node->stab, "@expr/neg", NUMBER_OBJ, LITERAL_TYPE);
                        d->type = LITERAL_TYPE;
                        d->cate = NUMBER_OBJ;
                        d->initval = -r->initval;
                    } else {
                        d = symalloc(node->stab, "@expr/neg", TEMP_OBJ, r->type);
                        emit2(NEG_OP, d, r);
                    }
                    break;
                case NOP_ADDOP:
                    d = r;
                    break;
                default:
                    unlikely();
            }
            continue;
        }

        switch (t->kind) {
            case NOP_ADDOP:
            case ADD_ADDOP:
                e = d;
                d = symalloc(node->stab, "@expr/add", TEMP_OBJ, e->type);
                emit3(ADD_OP, d, e, r);
                break;
            case MINUS_ADDOP:
            case NEG_ADDOP:
                e = d;
                d = symalloc(node->stab, "@expr/sub", TEMP_OBJ, e->type);
                emit3(SUB_OP, d, e, r);
                break;
            default:
                unlikely();
        }
    }
    return d;
}

static syment_t* gen_term(term_node_t *node) {
    term_node_t *t;
    syment_t *d, *r, *e;
    d = r = e = NULL;
    for (t = node; t; t = t->next) {
        r = gen_factor(t->fp);
        if (!d) {
            if (t->kind != NOP_MULTOP) {
                unlikely();
            }
            d = r;
            continue;
        }
        switch (t->kind) {
            case NOP_MULTOP:
            case MULT_MULTOP:
                e = d;
                d = symalloc(node->stab, "@term/mul", TEMP_OBJ, e->type);
                emit3(MUL_OP, d, e, r);
                break;
            case DIV_MULTOP:
                e = d;
                d = symalloc(node->stab, "@term/div", TEMP_OBJ, e->type);
                emit3(DIV_OP, d, e, r);
                break;
            default:
                unlikely();
        }
    }
    return d;
}

static syment_t* gen_factor(factor_node_t *node) {
    syment_t *d, *r, *e;
    d = r = e = NULL;
    switch (node->kind) {
        case ID_FACTOR:
            d = node->idp->symbol;
            break;
        case ARRAY_FACTOR:
            r = node->idp->symbol;
            e = gen_expr(node->ep);
            d = symalloc(node->stab, "@factor/array", TEMP_OBJ, r->type);
            emit3(LOAD_ARRAY_OP, d, r, e);
            break;
        case UNSIGN_FACTOR:
            d = symalloc(node->stab, "@factor/usi", NUMBER_OBJ, LITERAL_TYPE);
            d->initval = node->value;
            break;
        case CHAR_FACTOR:
            d = symalloc(node->stab, "@factor/char", NUMBER_OBJ, CHAR_TYPE);
            d->initval = node->value;
            break;
        case EXPR_FACTOR:
            d = gen_expr(node->ep);
            break;
        case FUNCALL_FACTOR:
            d = gen_fcall_stmt(node->fcsp);
            break;
        default:
            unlikely();
    }
    return d;
}

static syment_t* gen_fcall_stmt(fcall_stmt_node_t *node) {
    syment_t *d, *e;
    e = node->idp->symbol;
    d = symalloc(node->stab, "@fcall/ret", TEMP_OBJ, e->type);
    gen_arg_list(node->alp);
    emit2(CALL_OP, d, e);
    arg_list_node_t *t;
    for (t = node->alp; t; t = t->next) {
        emit1(POP_OP, NULL);
    }
    return d;
}

static void gen_cond(cond_node_t *node, syment_t *label) {
    syment_t *r, *s;
    r = gen_expr(node->lep);
    s = gen_expr(node->rep);
    switch (node->kind) {
        case EQU_RELA:
            emit3(BRANCH_EQU_OP, label, r, s);
            break;
        case NEQ_RELA:
            emit3(BRANCH_NEQ_OP, label, r, s);
            break;
        case GTT_RELA:
            emit3(BRANCH_GTT_OP, label, r, s);
            break;
        case GEQ_RELA:
            emit3(BRANCH_GEQ_OP, label, r, s);
            break;
        case LST_RELA:
            emit3(BRANCH_LST_OP, label, r, s);
            break;
        case LEQ_RELA:
            emit3(BRANCH_LEQ_OP, label, r, s);
            break;
    }
}

static void gen_arg_list(arg_list_node_t *node) {
    if (!node) {
        return;
    }
    arg_list_node_t *t = node;

    // Push arguments in reverse order
    gen_arg_list(t->next);

    syment_t *d = NULL, *r = NULL;
    switch (t->refsym->cate) {
        case BY_VALUE_OBJ:
            d = gen_expr(t->ep);
            emit1(PUSH_VAL_OP, d);
            break;
        case BY_REFERENCE_OBJ:
            d = t->argsym;
            switch (t->argsym->cate) {
                case VARIABLE_OBJ:
                    emit2(PUSH_ADDR_OP, d, NULL);
                    break;
                case ARRAY_OBJ:
                    r = gen_expr(t->idx);
                    emit2(PUSH_ADDR_OP, d, r);
                    break;
                default:
                    unlikely();
            }
            break;
        default:
            unlikely();
    }
}

void genir(pgm_node_t *pgm) {
    gen_pgm(pgm);
    chkerr("generate fail and exit.");
    phase = CODE_GEN;
}
