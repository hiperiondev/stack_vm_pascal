/*
 * @ parse.h
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

#ifndef _PARSE_H_
#define _PARSE_H_

#include "global.h"
#include "scan.h"

// Declaration of a bundle of tree node
typedef struct _pgm_node pgm_node_t;
typedef struct _block_node block_node_t;
typedef struct _const_dec_node const_dec_node_t;
typedef struct _const_def_node const_def_node_t;
typedef struct _var_dec_node var_dec_node_t;
typedef struct _var_def_node var_def_node_t;
typedef struct _pf_dec_list_node pf_dec_list_node_t;
typedef struct _proc_dec_node proc_dec_node_t;
typedef struct _proc_def_node proc_def_node_t;
typedef struct _proc_head_node proc_head_node_t;
typedef struct _fun_dec_node fun_dec_node_t;
typedef struct _fun_def_node fun_def_node_t;
typedef struct _fun_head_node fun_head_node_t;
typedef struct _stmt_node stmt_node_t;
typedef struct _assign_stmt_node assign_stmt_node_t;
typedef struct _if_stmt_node if_stmt_node_t;
typedef struct _repe_stmt_node repe_stmt_node_t;
typedef struct _for_stmt_node for_stmt_node_t;
typedef struct _pcall_stmt_node pcall_stmt_node_t;
typedef struct _fcall_stmt_node fcall_stmt_node_t;
typedef struct _comp_stmt_node comp_stmt_node_t;
typedef struct _read_stmt_node read_stmt_node_t;
typedef struct _write_stmt_node write_stmt_node_t;
typedef struct _expr_node expr_node_t;
typedef struct _term_node term_node_t;
typedef struct _factor_node factor_node_t;
typedef struct _cond_node cond_node_t;
typedef struct _ident_node ident_node_t;
typedef struct _para_list_node para_list_node_t;
typedef struct _para_def_node para_def_node_t;
typedef struct _arg_list_node arg_list_node_t;

extern int nidcnt;

// Create New Node
#define NEWNODE(s, v)     \
		INITMEM(s, v);    \
		v->nid = ++nidcnt

// use like:
//   if (TOKANY(a, b, c, ...)) { ... }
#define TOKANY(a)                  (currtok == (a))
#define TOKANY2(a, b)              (currtok == (a) || currtok == (b))
#define TOKANY3(a, b, c)           (currtok == (a) || currtok == (b) || currtok == (c))
#define TOKANY4(a, b, c, d)        (currtok == (a) || currtok == (b) || currtok == (c) || currtok == (d))
#define TOKANY5(a, b, c, d, e)     (currtok == (a) || currtok == (b) || currtok == (c) || currtok == (d) || currtok == (e))
#define TOKANY6(a, b, c, d, e, f)  (currtok == (a) || currtok == (b) || currtok == (c) || currtok == (d) || currtok == (e) || currtok == (f))

// ID read mode, for parse_ident()
typedef enum _idreadmode_enum {
    READCURR, //
    READPREV  //
} idreadmode_t;

// syntax analysis, produce the syntax tree
void parse(pgm_node_t **pgm);

#endif /* _PARSE_H_ */
