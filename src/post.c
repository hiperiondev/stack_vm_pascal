/*
 * @ post.c
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

#include "post.h"
#include "common.h"
#include "global.h"
#include "limits.h"
#include "util.h"

void post_nasm(void) {
    if (!chkcmd("nasm")) {
        panic("nasm not installed.\n");
    }

    char cmd[MAXSTRBUF];
    sprintf(cmd, "nasm -f elf -o %s %s", PL0E_OBJECT, PL0E_ASSEM);
    dbg("%s\n", cmd);

    errnum = system(cmd);
    chkerr("post_nasm fail and exit.");
    phase = LINK;

    msg("assemble object file %s\n", PL0E_OBJECT);
}

void post_link(void) {
    if (!chkcmd("ld")) {
        panic("ld not installed.\n");
    }

    char cmd[MAXSTRBUF];
    sprintf(cmd, "ld -m elf_i386 -o %s %s", PL0E_TARGET, PL0E_OBJECT);
    dbg("%s\n", cmd);

    errnum = system(cmd);
    chkerr("post_link fail and exit.");
    phase = SUCCESS;

    msg("link target file %s\n", PL0E_TARGET);
}

void post_clean(void) {
    if (!PL0E_OPT_KEEP_NASM_FILE) {
        remove(PL0E_ASSEM);
        msg("remove file %s\n", PL0E_ASSEM);
    }

    if (!PL0E_OPT_KEEP_OBJECT_FILE) {
        remove(PL0E_OBJECT);
        msg("remove file %s\n", PL0E_OBJECT);
    }
}
