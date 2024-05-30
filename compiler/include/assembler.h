/*
 * @assembler.h
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

#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#define MAXSTRINGLEN 8192

typedef struct fn_args_s {
    char label[1024];
    char name[1024];
    uint8_t category;
    uint8_t type;
} fn_args_t;

typedef struct fn_locales_s {
    char label[1024];
    char name[1024];
    uint8_t category;
    uint8_t type;
} fn_locales_t;

typedef struct fn_temps_s {
    char label[1024];
    char name[1024];
    uint8_t category;
    uint8_t type;
} fn_temps_t;

typedef struct fn_literals_s {
    char label[1024];
    long int value;
} fn_literals_t;

typedef struct fn_strings_s {
    char label[1024];
    char value[MAXSTRINGLEN];
} fn_strings_t;

typedef struct fn_elements_s {
    char name[1024];

    fn_args_t *args;
    fn_locales_t *locales;
    fn_temps_t *temps;
    fn_literals_t *literals;
    fn_strings_t *strings;

    long int args_qty;
    long int locales_qty;
    long int temps_qty;
    long int literals_qty;
    long int strings_qty;
} fn_elements_t;

extern fn_elements_t *fn_elements;
extern long int fn_elements_qty;

void genasm(char **asm_prg);
void print_fn_elements(void);
void free_asm(void);

#endif /* _ASSEMBLER_H_ */
