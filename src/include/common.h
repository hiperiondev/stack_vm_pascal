/*
 * @
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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

extern void **memtrack;
extern unsigned long memtrack_qty;

// Initialize struct, allocate memory
//     INITMEM(s: struct, v: variable, struct pointer)
#define INITMEM(s, v)                                                     \
		v = (s*)calloc(1, sizeof(s));                                     \
		memtrack = realloc(memtrack, (memtrack_qty + 1) * sizeof(void*)); \
		memtrack[memtrack_qty] = (void*)v;                                \
		++memtrack_qty;                                                   \
		if (v == NULL) {                                                  \
			panic("OUT_OF_MEMORY");                                       \
		}

// Compiling Phase
typedef enum _phase_enum {
    /* 0 */INIT,
    /* 1 */LEXICAL,
    /* 2 */SYNTAX,
    /* 3 */SEMANTIC,
    /* 4 */IR,
    /* 5 */CODE_GEN,
    /* 6 */ASSEMBLE,
    /* 7 */LINK,
    /* 8 */SUCCESS
} phase_t;

#endif /* _COMMON_H_ */
