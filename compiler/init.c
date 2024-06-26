/*
 * @init.c
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

#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "debug.h"
#include "error.h"
#include "global.h"
#include "limits.h"
#include "util.h"
#include "version.h"

// constants
char PL0E_NAME[MAXSTRLEN] = "stack_vm_pascal";
char PL0E_VERSION[MAXSTRLEN] = COMPILER_VERSION(COMPILER_VERSION_MAYOR,COMPILER_VERSION_MINOR,COMPILER_VERSION_PATCH);
char PL0E_INPUT[MAXSTRLEN] = "input.pas";
char PL0E_ASSEM[MAXSTRLEN] = "input.s";
char PL0E_OBJECT[MAXSTRLEN] = "input.o";
char PL0E_TARGET[MAXSTRLEN] = "a.out";

// options
bool PL0E_OPT_QUIET = false;
bool PL0E_OPT_VERBOSE = false;
bool PL0E_OPT_SET_TARGET_NAME = false;

// debug
bool echo = false;
bool silent = false;

// files
FILE *source = NULL;
FILE *asmble = NULL;

phase_t phase = INIT;
int errnum = 0;

void pl0c_read_args(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; ++i) {
        if (!strcmp("-q", argv[i])) {
            PL0E_OPT_QUIET = true;
            echo = false;
            silent = true;
            continue;
        }
        if (!strcmp("-v", argv[i])) {
            PL0E_OPT_VERBOSE = true;
            echo = true;
            silent = false;
            continue;
        }
        if (!strcmp("-o", argv[i])) {
            PL0E_OPT_SET_TARGET_NAME = true;
            i++;
            if (i == argc) {
                panic("should give target file name after -o");
            }
            strcpy(PL0E_TARGET, argv[i]);
            continue;
        }
        if (strlen(argv[i]) > 0 && argv[i][0] != '-') {
            strcpy(PL0E_INPUT, argv[i]);
        }
    }
    dbg("current input file %s\n", PL0E_INPUT);
}

void pl0c_startup_message() {
    msg("; compiler %s start, version %s\n", PL0E_NAME, PL0E_VERSION);
}

void pl0c_init_file() {
    if (access(PL0E_INPUT, R_OK)) {
        msg("cannot read file %s\n", PL0E_INPUT);
        exit(EARGMT);
    }

    strcpy(PL0E_ASSEM, PL0E_INPUT);
    chgsuf(PL0E_ASSEM, ".s", ".pas");
    strcpy(PL0E_OBJECT, PL0E_INPUT);
    chgsuf(PL0E_OBJECT, ".o", ".pas");
    if (!PL0E_OPT_SET_TARGET_NAME) {
        strcpy(PL0E_TARGET, PL0E_INPUT);
        chgsuf(PL0E_TARGET, ".run", ".pas");
    }

    source = fopen(PL0E_INPUT, "r");
    if (!source) {
        panic("SOURCE_FILE_NOT_FOUND");
    }
    msg("; file %s\n", PL0E_INPUT);
}

// init
void init(int argc, char *argv[]) {
    pl0c_read_args(argc, argv);

    pl0c_startup_message();

    pl0c_init_file();

    chkerr("init fail and exit.");
    phase = LEXICAL;
}
