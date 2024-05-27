/*
 * @util.h
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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>

#include "limits.h"
#include "common.h"

// common string buffer
extern char prtbuf[MAXSTRBUF];

// strcat + sprintf
#define appendf(s, fmt, args...)      \
		sprintf(prtbuf, fmt, ##args); \
		strcat(s, prtbuf)

void strcopy(char *d, char *s);
void chgsuf(char *str, char *to, char *from);
char* itoa(int num);
bool chkcmd(char *cmd);

// bitset constants
// bit shift
#define BITSHIFT 5
// elements of bit shift, which is 2^BITSHIFT
#define BITSIZE (1 << BITSHIFT)

// unsigned 64 bit int
typedef unsigned int bits_t;
typedef char bin_t[BITSIZE + 1];

//
//   HIGH                   LOW
//  index xxxxxxxxxxxxxxxxx
//        |---------||----|
//             |       |
//             |        `- offset in the bit (LSB BITSHIFT bits in index)
//             `---------- position in array (HSB (BITSIZE-BITSHIFT) bits in index)
//
// offset of index(i)
#define OFF(i)  (((bits_t)i) & (~((~0) << BITSHIFT)))
// position of index(i)
#define POS(i)  ((((bits_t)i) & ((~0) << BITSHIFT)) >> BITSHIFT)
// mask on element
#define MASK(i) (1L << OFF(i))

void bconv(bin_t str, bits_t b);
void bset(bits_t bits[], int i);
void bclr(bits_t bits[], int i);
bool bget(bits_t bits[], int i);
void bclrall(bits_t bits[], int n);
void bsetall(bits_t bits[], int n);
bool bsame(bits_t a[], bits_t b[], int n);
void bdup(bits_t des[], bits_t src[], int n);
void bunion(bits_t r[], bits_t a[], bits_t b[], int n);
void bsub(bits_t r[], bits_t a[], bits_t b[], int n);

#endif /* _UTIL_H_ */
