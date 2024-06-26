/*
 * @anlysis.c
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

#include "error.h"
#include "anlysis.h"
#include "global.h"
#include "debug.h"
#include "limits.h"
#include "util.h"
#include "syntax.h"
#include "parse.h"
#include "symtab.h"

extern void **memtrack;
extern unsigned long memtrack_qty;

static void anlys_pgm(pgm_node_t *node);
static void anlys_const_decf(const_dec_node_t *node);
static void anlys_var_decf(var_dec_node_t *node);
static void anlys_pf_dec_list(pf_dec_list_node_t *node);
static void anlys_proc_decf(proc_dec_node_t *node);
static void anlys_proc_head(proc_head_node_t *node);
static void anlys_fun_decf(fun_dec_node_t *node);
static void anlys_fun_head(fun_head_node_t *node);
static param_t* anlys_para_list(para_list_node_t *node);
static void anlys_comp_stmt(comp_stmt_node_t *node);
static void anlys_stmt(stmt_node_t *node);
static void anlys_assign_stmt(assign_stmt_node_t *node);
static void anlys_if_stmt(if_stmt_node_t *node);
static void anlys_repe_stmt(repe_stmt_node_t *node);
static void anlys_for_stmt(for_stmt_node_t *node);
static void anlys_pcall_stmt(pcall_stmt_node_t *node);
static void anlys_read_stmt(read_stmt_node_t *node);
static void anlys_write_stmt(write_stmt_node_t *node);
static void anlys_expr(expr_node_t *node);
static void anlys_term(term_node_t *node);
static void anlys_factor(factor_node_t *node);
static void anlys_fcall_stmt(fcall_stmt_node_t *node);
static void anlys_cond(cond_node_t *node);
static void anlys_arg_list(syment_t *sign, arg_list_node_t *node);

static type_t infer_expr_type(expr_node_t *node);
static type_t infer_term_type(term_node_t *node);
static type_t infer_factor_type(factor_node_t *node);

char *typerepr[6] = {
        [0] = "_V", // VOID_TYPE
        [1] = "_I", // INT_TYPE
        [2] = "_U", // UINT_TYPE
        [3] = "_C", // CHAR_TYPE
        [4] = "_S", // STR_TYPE
        [5] = "_L", // LITERAL_TYPE
};

static type_t infer_expr_type(expr_node_t *node) {
    type_t ltyp, rtyp;
    ltyp = infer_term_type(node->tp);
    expr_node_t *t;
    for (t = node->next; t; t = t->next) {
        rtyp = infer_term_type(t->tp);
        if (ltyp == CHAR_TYPE && rtyp == CHAR_TYPE) {
            ltyp = CHAR_TYPE;
        } else if (ltyp == INT_TYPE && rtyp == INT_TYPE) {
            ltyp = INT_TYPE;
        } else if (ltyp == UINT_TYPE && rtyp == UINT_TYPE) {
            ltyp = UINT_TYPE;
        } else if (ltyp == LITERAL_TYPE && rtyp == LITERAL_TYPE) {
            ltyp = LITERAL_TYPE;
        }
    }
    return ltyp;
}

static type_t infer_term_type(term_node_t *node) {
    type_t ltyp, rtyp;
    ltyp = infer_factor_type(node->fp);
    term_node_t *t;
    for (t = node->next; t; t = t->next) {
        rtyp = infer_factor_type(t->fp);
        if (ltyp == CHAR_TYPE && rtyp == CHAR_TYPE) {
            ltyp = CHAR_TYPE;
        } else if (ltyp == INT_TYPE && rtyp == INT_TYPE) {
            ltyp = INT_TYPE;
        } else if (ltyp == UINT_TYPE && rtyp == UINT_TYPE) {
            ltyp = UINT_TYPE;
        }  else if (ltyp == LITERAL_TYPE && rtyp == LITERAL_TYPE) {
            ltyp = LITERAL_TYPE;
        }
    }
    return ltyp;
}

static type_t infer_factor_type(factor_node_t *node) {
    ident_node_t *idp;
    syment_t *e = NULL;
    switch (node->kind) {
        case ID_FACTOR:
            nevernil(node->idp);
            idp = node->idp;
            e = symfind(idp->name);
            if (!e) {
                giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
            }
            return e->type;
        case ARRAY_FACTOR:
            nevernil(node->idp);
            idp = node->idp;
            e = symfind(idp->name);
            if (!e) {
                giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
            }
            if (e->cate != ARRAY_OBJ) {
                giveup(ERTYPE, "line %d: symbol %s type is not array.", idp->line, idp->name);
            }
            return e->type;
        case UNSIGN_FACTOR:
            return LITERAL_TYPE;
        case CHAR_FACTOR:
            return CHAR_TYPE;
        case EXPR_FACTOR:
            return infer_expr_type(node->ep);
        case FUNCALL_FACTOR:
            panic("FUNCALL_IN_INFERENCE");
            break;
        default:
            unlikely();
    }

    return 0;
}

static void make_symkey(char *key, param_t *phead) {
    param_t *p = NULL;
    for (p = phead; p; p = p->next) {
        strncat(key, typerepr[p->symbol->type], MAXSTRLEN - 1);
    }
}

static void make_symkey2(char *key, arg_list_node_t *arglist, bool lit_utoi) {
    arg_list_node_t *a = NULL;
    for (a = arglist; a; a = a->next) {
        type_t typ = infer_expr_type(a->ep);
        if (typ == LITERAL_TYPE) {
            if (a->ep->kind == NEG_ADDOP) {
                typ = INT_TYPE;
            } else {
                if (lit_utoi) {
                    typ = INT_TYPE;
                } else {
                    typ = UINT_TYPE;
                }
            }
        }

        strncat(key, typerepr[typ], MAXSTRLEN - 1);
    }
}

static void anlys_pgm(pgm_node_t *node) {
    scope_entry(MAINFUNC);

    syment_t *e = syminit(node->entry);
    node->entry->symbol = e;
    node->entry->symbol->scope = scope_top();

    nevernil(node->bp);
    block_node_t *b = node->bp;
    anlys_const_decf(b->cdp);
    anlys_var_decf(b->vdp);
    anlys_pf_dec_list(b->pfdlp);
    anlys_comp_stmt(b->csp);

    scope_exit();
}

static void anlys_const_decf(const_dec_node_t *node) {
    const_dec_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->cdp);
        nevernil(t->cdp->idp);
        ident_node_t *idp = t->cdp->idp;
        syment_t *e = symget(idp->name);
        if (e) {
            rescue(DUPSYM, "line %d: const %s already declared.", idp->line, idp->name);
        } else {
            e = syminit(idp);
        }
        idp->symbol = e;
    }
}

static void anlys_var_decf(var_dec_node_t *node) {
    var_dec_node_t *t;
    var_def_node_t *p;
    for (t = node; t; t = t->next) {
        for (p = t->vdp; p; p = p->next) {
            nevernil(p->idp);
            ident_node_t *idp = p->idp;
            syment_t *e = symget(idp->name);
            if (e) {
                rescue(DUPSYM, "line %d: variable %s already declared.", idp->line, idp->name);
            } else {
                e = syminit(idp);
            }
            idp->symbol = e;
        }
    }
}

static void anlys_pf_dec_list(pf_dec_list_node_t *node) {
    pf_dec_list_node_t *t;
    for (t = node; t; t = t->next) {
        switch (t->kind) {
            case PROC_PFDEC:
                anlys_proc_decf(t->pdp);
                break;
            case FUN_PFDEC:
                anlys_fun_decf(t->fdp);
                break;
            default:
                unlikely();
        }
    }
}

static void anlys_proc_decf(proc_dec_node_t *node) {
    proc_dec_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->pdp);
        nevernil(t->pdp->php);
        anlys_proc_head(t->pdp->php);

        nevernil(t->pdp->bp);
        block_node_t *b = t->pdp->bp;

        anlys_const_decf(b->cdp);
        anlys_var_decf(b->vdp);
        anlys_pf_dec_list(b->pfdlp);
        anlys_comp_stmt(b->csp);

        scope_exit();
    }
}

static void anlys_proc_head(proc_head_node_t *node) {
    proc_head_node_t *t = node;

    nevernil(t->idp);
    ident_node_t *idp = t->idp;

    symtab_t *parent = scope_top();
    symtab_t *scope = scope_entry(idp->name);

    // analysis parameters
    param_t *phead = NULL;
    if (t->plp) {
        phead = anlys_para_list(t->plp);
    }

    // construct procedure name
    char pname[MAXSTRLEN];
    strncpy(pname, idp->name, MAXSTRLEN);
    make_symkey(pname, phead);

    syment_t *e = symget2(parent, pname);
    if (e) {
        rescue(DUPSYM, "line %d: procedure %s already declared.", idp->line, idp->name);
    } else {
        e = syminit2(parent, idp, pname);
    }

    scope->funcsym = e;
    e->scope = scope;
    e->phead = phead;
    idp->symbol = e;
}

static void anlys_fun_decf(fun_dec_node_t *node) {
    fun_dec_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->fdp);
        nevernil(t->fdp->fhp);
        anlys_fun_head(t->fdp->fhp);

        nevernil(t->fdp->bp);
        block_node_t *b = t->fdp->bp;

        anlys_const_decf(b->cdp);
        anlys_var_decf(b->vdp);
        anlys_pf_dec_list(b->pfdlp);
        anlys_comp_stmt(b->csp);

        scope_exit();
    }
}

static void anlys_fun_head(fun_head_node_t *node) {
    fun_head_node_t *t = node;

    nevernil(t->idp);
    ident_node_t *idp = t->idp;

    symtab_t *parent = scope_top();
    symtab_t *scope = scope_entry(idp->name);

    // analysis parameters
    param_t *phead = NULL;
    if (t->plp) {
        phead = anlys_para_list(t->plp);
    }

    // construct function name
    char fname[MAXSTRLEN];
    strncpy(fname, idp->name, MAXSTRLEN);
    make_symkey(fname, phead);

    syment_t *e = symget2(parent, fname);
    if (e) {
        rescue(DUPSYM, "line %d: function %s already declared.", idp->line, idp->name);
    } else {
        e = syminit2(parent, idp, fname);
    }

    scope->funcsym = e;
    e->scope = scope;
    e->phead = phead;
    idp->symbol = e;
}

static param_t* anlys_para_list(para_list_node_t *node) {
    param_t *phead = NULL, *ptail = NULL;
    para_list_node_t *t;
    para_def_node_t *p;
    for (t = node; t; t = t->next) {
        for (p = t->pdp; p; p = p->next) {
            nevernil(p->idp);
            ident_node_t *idp = p->idp;
            syment_t *e = symget(idp->name);
            if (e) {
                rescue(DUPSYM, "line %d: parameter %s already declared.", idp->line, idp->name);
            } else {
                e = syminit(idp);
            }
            idp->symbol = e;

            param_t *param;
            NEWPARAM(param);
            param->symbol = e;

            if (!ptail) {
                ptail = param;
                phead = param;
            } else {
                ptail->next = param;
                ptail = param;
            }
        }
    }

    return phead;
}

static void anlys_comp_stmt(comp_stmt_node_t *node) {
    comp_stmt_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->sp);
        anlys_stmt(t->sp);
    }
}

static void anlys_stmt(stmt_node_t *node) {
    switch (node->kind) {
        case ASSGIN_STMT:
            anlys_assign_stmt(node->asp);
            break;
        case IF_STMT:
            anlys_if_stmt(node->ifp);
            break;
        case REPEAT_STMT:
            anlys_repe_stmt(node->rpp);
            break;
        case FOR_STMT:
            anlys_for_stmt(node->frp);
            break;
        case PCALL_STMT:
            anlys_pcall_stmt(node->pcp);
            break;
        case COMP_STMT:
            anlys_comp_stmt(node->cpp);
            break;
        case READ_STMT:
            anlys_read_stmt(node->rdp);
            break;
        case WRITE_STMT:
            anlys_write_stmt(node->wtp);
            break;
        case NULL_STMT:
            break;
        default:
            unlikely();
    }
}

static void anlys_assign_stmt(assign_stmt_node_t *node) {
    syment_t *e;
    ident_node_t *idp = node->idp;

    if (!strcmp(idp->name, scope_top()->nspace)) {
        e = scope_top()->funcsym;
    } else {
        e = symfind(idp->name);
        if (!e) {
            giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
        }
    }

    idp->symbol = e;
    switch (node->kind) {
        case NORM_ASSGIN:
        case FUN_ASSGIN:
            anlys_expr(node->rep);
            break;
        case ARRAY_ASSGIN:
            anlys_expr(node->lep);
            anlys_expr(node->rep);
            break;
        default:
            unlikely();
    }
}

static void anlys_if_stmt(if_stmt_node_t *node) {
    node->stab = scope_top();
    anlys_cond(node->cp);
    if (node->ep) {
        anlys_stmt(node->ep);
    }
    anlys_stmt(node->tp);
}

static void anlys_repe_stmt(repe_stmt_node_t *node) {
    node->stab = scope_top();
    anlys_stmt(node->sp);
    anlys_cond(node->cp);
}

static void anlys_for_stmt(for_stmt_node_t *node) {
    node->stab = scope_top();
    anlys_expr(node->lep);
    anlys_expr(node->rep);

    ident_node_t *idp = node->idp;
    syment_t *e = symfind(idp->name);
    if (!e) {
        giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
    }
    idp->symbol = e;

    switch (node->kind) {
        case TO_FOR:
        case DOWNTO_FOR:
            anlys_stmt(node->sp);
            break;
        default:
            unlikely();
    }
}

static void anlys_pcall_stmt(pcall_stmt_node_t *node) {
    syment_t *e;

    ident_node_t *idp = node->idp;
    char pname[MAXSTRLEN];

    strncpy(pname, idp->name, MAXSTRLEN);
    make_symkey2(pname, node->alp, false);
    e = symfind(pname);

    if (!e) {
        strncpy(pname, idp->name, MAXSTRLEN);
        make_symkey2(pname, node->alp, true);
        e = symfind(pname);
    }

    if (!e) {
        giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
    }
    if (e->cate != PROC_OBJ) {
        giveup(BADSYM, "line %d: procedure %s not found.", idp->line, idp->name);
    }
    idp->symbol = e;

    if (node->alp) {
        anlys_arg_list(e, node->alp);
    }
}

static void anlys_read_stmt(read_stmt_node_t *node) {
    read_stmt_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->idp);
        ident_node_t *idp = t->idp;
        syment_t *e = symfind(idp->name);
        if (!e) {
            giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
        }
        idp->symbol = e;
    }
}

static void anlys_write_stmt(write_stmt_node_t *node) {
    node->stab = scope_top();
    switch (node->type) {
        case STR_WRITE:
            break;
        case ID_WRITE:
        case STRID_WRITE:
            anlys_expr(node->ep);
            break;
        default:
            unlikely();
    }
}

static void anlys_expr(expr_node_t *node) {
    node->stab = scope_top();
    expr_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->tp);
        anlys_term(t->tp);
    }
}

static void anlys_term(term_node_t *node) {
    node->stab = scope_top();
    term_node_t *t;
    for (t = node; t; t = t->next) {
        nevernil(t->fp);
        anlys_factor(t->fp);
    }
}

static void anlys_factor(factor_node_t *node) {
    node->stab = scope_top();
    ident_node_t *idp;
    syment_t *e;
    switch (node->kind) {
        case ID_FACTOR:
            nevernil(node->idp);
            idp = node->idp;
            e = symfind(idp->name);
            if (!e) {
                giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
            }
            switch (e->cate) {
                case CONSTANT_OBJ:
                case VARIABLE_OBJ:
                case TEMP_OBJ:
                case BY_VALUE_OBJ:
                case BY_REFERENCE_OBJ:
                    break;
                default:
                    giveup(BADCTG, "line %d: symbol %s category is bad.", idp->line, idp->name);
            }
            idp->symbol = e;
            break;
        case ARRAY_FACTOR:
            nevernil(node->idp);
            idp = node->idp;
            e = symfind(idp->name);
            if (!e) {
                giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
            }
            if (e->cate != ARRAY_OBJ) {
                giveup(ERTYPE, "line %d: symbol %s type is not array.", idp->line, idp->name);
            }
            idp->symbol = e;

            nevernil(node->ep);
            anlys_expr(node->ep);
            break;
        case UNSIGN_FACTOR:
            break;
        case CHAR_FACTOR:
            break;
        case EXPR_FACTOR:
            nevernil(node->ep);
            anlys_expr(node->ep);
            break;
        case FUNCALL_FACTOR:
            nevernil(node->fcsp);
            anlys_fcall_stmt(node->fcsp);
            break;
        default:
            unlikely();
    }
}

static void anlys_fcall_stmt(fcall_stmt_node_t *node) {
    syment_t *e;

    node->stab = scope_top();
    nevernil(node->idp);

    ident_node_t *idp = node->idp;
    char fname[MAXSTRLEN];

    strncpy(fname, idp->name, MAXSTRLEN);
    make_symkey2(fname, node->alp, false);
    e = symfind(fname);

    if (!e) {
        strncpy(fname, idp->name, MAXSTRLEN);
        make_symkey2(fname, node->alp, true);
        e = symfind(fname);
    }

    if (!e) {
        giveup(BADSYM, "line %d: function %s not found.", idp->line, idp->name);
    }
    if (e->cate != FUNCTION_OBJ) {
        giveup(ERTYPE, "line %d: symbol %s type is not function.", idp->line, idp->name);
    }
    idp->symbol = e;

    if (node->alp) {
        anlys_arg_list(e, node->alp);
    }
}

static void anlys_cond(cond_node_t *node) {
    nevernil(node->lep);
    anlys_expr(node->lep);
    nevernil(node->rep);
    anlys_expr(node->rep);
}

static void anlys_arg_list(syment_t *sign, arg_list_node_t *node) {
    arg_list_node_t *t = node;
    param_t *p = sign->phead;
    int pos = 0;
    for (; t && p; t = t->next, p = p->next) {
        pos++;
        syment_t *e, *a;
        e = p->symbol;
        switch (e->cate) {
            case BY_VALUE_OBJ:
                nevernil(t->ep);
                anlys_expr(t->ep);
                t->refsym = e;
                break;
            case BY_REFERENCE_OBJ: // var, arr[exp]
                if (!t->ep) {
                    goto referr;
                }
                if (t->ep->kind != NOP_ADDOP) {
                    goto referr;
                }
                expr_node_t *ep = t->ep;
                if (ep->next) {
                    goto referr;
                }
                if (!ep->tp || ep->tp->kind != NOP_MULTOP) {
                    goto referr;
                }
                term_node_t *tp = ep->tp;
                if (!tp->fp) {
                    goto referr;
                }

                factor_node_t *fp = tp->fp;
                ident_node_t *idp;
                if (fp->kind == ID_FACTOR || fp->kind == ARRAY_FACTOR) {
                    idp = fp->idp;
                    t->idx = fp->ep;
                    anlys_factor(fp);
                    goto refok;
                }
referr:
                giveup(BADREF, "line %d: %s() arg%d has bad reference.", sign->lineno, sign->name, pos);
                continue;
refok:
                a = symfind(idp->name);
                if (!a) {
                    giveup(BADSYM, "line %d: symbol %s not found.", idp->line, idp->name);
                }
                if (fp->kind == ID_FACTOR && a->cate != VARIABLE_OBJ) {
                    giveup(OBJREF, "line %d: %s() arg%d is not variable object.", idp->line, idp->name, pos);
                }
                if (fp->kind == ARRAY_FACTOR && a->cate != ARRAY_OBJ) {
                    giveup(OBJREF, "line %d: %s() arg%d is not array object.", idp->line, idp->name, pos);
                }
                t->argsym = idp->symbol = a;
                t->refsym = e;
                break;
            default:
                unlikely();
        }
    }

    if (t || p) {
        giveup(BADLEN, "line %d: %s(...) arguments and parameters length not equal.", sign->lineno, sign->name);
    }
}

void analysis(pgm_node_t *pgm) {
    anlys_pgm(pgm);
    chkerr("analysis fail and exit.");
    phase = IR;
}
