/*
 * @error.c
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

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "debug.h"
#include "error.h"

void quit(char *file, int line, const char *func, int errno, char *msg) {
    char *prefix = "QUIT";
    if (errno == EABORT) {
        prefix = "ABORT";
    } else if (errno == EPANIC) {
        prefix = "PANIC";
    }

    fprintf(stderr, "%s: %s:%d %s(): %s\n", prefix, file, line, func, msg);

    exit(errno);
}
