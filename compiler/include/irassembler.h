/*
 * @irassembler.h
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

#ifndef _IRASSEMBLER_H_
#define _IRASSEMBLER_H_

#define MAXSTRINGLEN 8192

typedef struct fn_ir_args_s {
       char label[1024];
       char name[1024];
    uint8_t category;
    uint8_t type;
} fn_ir_args_t;

typedef struct fn_ir_locales_s {
       char label[1024];
       char name[1024];
    uint8_t category;
    uint8_t type;
} fn_ir_locales_t;

typedef struct fn_ir_temps_s {
       char label[1024];
       char name[1024];
    uint8_t category;
    uint8_t type;
} fn_ir_temps_t;

typedef struct fn_ir_strings_s {
    char label[1024];
    char value[MAXSTRINGLEN];
} fn_ir_strings_t;

typedef struct fn_ir_elements_s {
             char name[1024];
             char label[1024];

        fn_ir_args_t *args;
     fn_ir_locales_t *locales;
       fn_ir_temps_t *temps;
     fn_ir_strings_t *strings;

         long int args_qty;
         long int locales_qty;
         long int temps_qty;
         long int literals_qty;
         long int strings_qty;
} fn_ir_elements_t;

typedef union irasm_value_u {
    long int number;
        char str[MAXSTRINGLEN];
} asm_value_t;

typedef struct irasm_result_s {
    uint8_t op;
    asm_value_t arg1;
    asm_value_t arg2;
    asm_value_t arg3;
    asm_value_t arg4;
    asm_value_t arg5;
    asm_value_t arg6;
    asm_value_t arg7;
    asm_value_t arg8;
} asm_result_t;

extern fn_ir_elements_t *fn_ir_elements;
extern long int fn_ir_elements_qty;

uint32_t gen_irasm(asm_result_t **irasm_result);
void print_ir_fn_elements(void);
void print_irasm(asm_result_t *irasm_result, uint32_t irasm_result_len);
void free_irasm(void);

#endif /* _IRASSEMBLER_H_ */
