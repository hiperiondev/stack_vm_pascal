/*
 * @ symtab.h
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

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "common.h"
#include "global.h"
#include "limits.h"
#include "parse.h"

#define MAXBUCKETS 16

typedef struct _sym_param_struct param_t;
typedef struct _sym_entry_struct syment_t;
typedef struct _sym_table_struct symtab_t;

// symbol category
typedef enum _sym_cate_enum {
    // Primary Object
    NOP_OBJ,   // 0x00
    CONST_OBJ, // 0x01
    VAR_OBJ,   // 0x02
    PROC_OBJ,  // 0x03
    FUN_OBJ,   // 0x04
    ARRAY_OBJ, // 0x05
    BYVAL_OBJ, // 0x06
    BYREF_OBJ, // 0x07

    // Additional
    TMP_OBJ,   // 0x08
    LABEL_OBJ, // 0x09
    NUM_OBJ,   // 0x0a
    STR_OBJ    // 0x0b
} cate_t;

// symbol type
typedef enum _sym_type_enum {
    VOID_TYPE, // 0x00
    INT_TYPE,  // 0x01
    UINT_TYPE, // 0x02
    CHAR_TYPE, // 0x03
    STR_TYPE   // 0x04
} type_t;

// signature for procedure and function
struct _sym_param_struct {
    syment_t *symbol;
    param_t *next;
};

struct _sym_entry_struct {
    int sid;               //
    char name[MAXSTRLEN];  // identifier name
    cate_t cate;           //
    type_t type;           //
    int initval;           // const value, initval value
    int arrlen;            //
    char str[MAXSTRLEN];   //
    param_t *phead;        //
    symtab_t *scope;       //
    char label[MAXSTRLEN]; // label for assemble codes
    int off;               // offset, for local variable stack mapping
    int lineno;            // referred line number
    symtab_t *stab;        // which symbol table
    syment_t *next;
};

struct _sym_table_struct {
    int tid; // symbol table ID

    // for function scope management
    int depth;		        // symbol table nested depth
    char nspace[MAXSTRLEN]; // namespace
    syment_t *funcsym;	    // current scope function/procedure symbol
    symtab_t *inner;	    // inner scope
    symtab_t *outer;	    // outer scope

    // for assembly stack mapping
    //    1. arguments values
    //    2. saved ebps
    //    3. return value
    //    4. local variables     (varoff)
    //    5. temporary variables (tmpoff)
    int argoff; // argument variable offset in total
    int varoff; // variable offset in total
    int tmpoff; // temporary variable offset in total

    // entries buckets
    syment_t buckets[MAXBUCKETS];
};

// Constructor
#define NEWPARAM(v) INITMEM(param_t, v)
#define NEWENTRY(v) INITMEM(syment_t, v)
#define NEWSTAB(v)  INITMEM(symtab_t, v)

// store all symbols
extern syment_t *syments[MAXSYMENT]; // map[sid]*syment_t
extern int sidcnt;		             // sid counter

// scope management
symtab_t* scope_entry(char *nspace);
symtab_t* scope_exit(void);
symtab_t* scope_top(void);
// symbol operator
void symadd(syment_t *entry);
void symadd2(symtab_t *stab, syment_t *entry);
// symget only search current scope, while symfind search all.
syment_t* symget(char *name);
syment_t* symget2(symtab_t *stab, char *name);
syment_t* symfind(char *name);
     void stabdump(void);
syment_t* syminit(ident_node_t *idp);
syment_t* syminit2(symtab_t *stab, ident_node_t *idp, char *key);
syment_t* symalloc(symtab_t *stab, char *name, cate_t cate, type_t type);

#endif /* _SYMTAB_H_ */
