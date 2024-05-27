/*
 * @error.h
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

#ifndef _ERROR_H_
#define _ERROR_H_

#define ERRTOK 100
#define DUPSYM 110
#define BADSYM 111
#define BADCTG 112
#define ERTYPE 113
#define BADLEN 114
#define BADREF 115
#define OBJREF 106
#define ENOCMD 995
#define EPANIC 996
#define EABORT 997
#define EARGMT 998

#define rescue(err, fmt, args...) \
		errnum = err;             \
		printf("ERROR: ");        \
		printf(fmt, ##args);      \
		printf("\n")

#define giveup(err, fmt, args...) \
		errnum = err;             \
		printf(fmt, ##args);      \
		printf("\n");             \
		exit(err)

#define chkerr(fmt)       \
		if (errnum > 0) { \
			printf(fmt);  \
			printf("\n"); \
			exit(errnum); \
		}

#endif /* _ERROR_H_ */
