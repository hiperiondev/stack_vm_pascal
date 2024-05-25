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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "error.h"

void quit(char *file, int line, const char *func, int errno, char *msg);

// print message
#define msg(fmt, args...)        \
		if (!silent) {           \
			printf(fmt, ##args); \
		}

// debug print message
#define dbg(fmt, args...)                                                     \
		if (echo) {                                                           \
			printf("%s:%d %s(): " fmt, __FILE__, __LINE__, __func__, ##args); \
		}

// panic function
#define panic(msg) quit(__FILE__, __LINE__, __func__, EPANIC, msg)

// debug unlikely case function
#define unlikely() quit(__FILE__, __LINE__, __func__, EABORT, "unlikely case")
#define nevernil(p)     \
		if (!p) {       \
			unlikely(); \
		}

#endif /* _DEBUG_H_ */
