/*
 * @ scan.c
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

#include <ctype.h>

#include "global.h"
#include "lexical.h"
#include "scan.h"

static int readc(bool peek);
static void unreadc(void);
static token_t getkw(char *s);

char tokbuf[MAXTOKSIZE + 1];
int toklineno;

// get next token
token_t gettok(void) {
    // token buffer index
    int i = 0;
    // current token
    token_t curr = 0;
    // whether save current character to tokbuf[...]
    bool save;

    // the state of our state machine
    state_t state = START;

    // the state machine main loop
    while (state != DONE) {
        int ch = readc(FALSE);
        save = TRUE;
        // state machine
        switch (state) {
            case START:
                if (isspace(ch)) {
                    save = FALSE;
                } else if (isdigit(ch)) {
                    state = INUNS;
                } else if (ch == '"') {
                    save = FALSE;
                    state = INSTR;
                } else if (ch == '\'') {
                    save = FALSE;
                    state = INCHA;
                } else if (ch == '{') {
                    save = FALSE;
                    state = INCMT;
                } else if (isalpha(ch)) {
                    state = INIDE;
                } else if (ch == ':') {
                    state = INCOM;
                } else if (ch == '>') {
                    state = INGRE;
                } else if (ch == '<') {
                    state = INLES;
                } else {
                    state = DONE;
                    switch (ch) {
                        case EOF:
                            save = FALSE;
                            curr = ENDFILE;
                            break;
                        case '.':
                            curr = SS_DOT;
                            break;
                        case '+':
                            curr = SS_PLUS;
                            break;
                        case '-':
                            curr = SS_MINUS;
                            break;
                        case '*':
                            curr = SS_STAR;
                            break;
                        case '/':
                            curr = SS_OVER;
                            break;
                        case '=':
                            curr = SS_EQU;
                            break;
                        case ',':
                            curr = SS_COMMA;
                            break;
                        case ';':
                            curr = SS_SEMI;
                            break;
                        case '(':
                            curr = SS_LPAR;
                            break;
                        case ')':
                            curr = SS_RPAR;
                            break;
                        case '[':
                            curr = SS_LBRA;
                            break;
                        case ']':
                            curr = SS_RBRA;
                            break;
                        case '{':
                            curr = SS_LBBR;
                            break;
                        case '}':
                            curr = SS_RBBR;
                            break;
                        default:
                            curr = ERROR;
                            break;
                    }
                }
                break;
            case INCMT: // in comment
                save = FALSE;
                if (ch == EOF) {
                    state = DONE;
                    curr = ENDFILE;
                } else if (ch == '}') {
                    state = START;
                }
                break;
            case INSTR: // in string
                if (ch == '"') {
                    state = DONE;
                    save = FALSE;
                    curr = MC_STR;
                } else if (isprint(ch)) {
                    // only allow printable character
                } else if (!isprint(ch)) {
                    panic("unprintable character");
                } else {
                    state = DONE;
                    if (ch == EOF) {
                        save = FALSE;
                        i = 0;
                        curr = ENDFILE;
                    }
                }
                break;
            case INCHA: // in character
                if (ch == '\'') {
                    state = DONE;
                    save = FALSE;
                    curr = MC_CH;
                } else if (isdigit(ch) || isalpha(ch)) {
                    // skip case
                } else {
                    if (ch == EOF) {
                        save = FALSE;
                        i = 0;
                        curr = ENDFILE;
                        state = DONE;
                    }
                }
                break;
            case INUNS: // in unsign number
                if (!isdigit(ch)) {
                    unreadc();
                    save = FALSE;
                    state = DONE;
                    curr = MC_UNS;
                }
                break;
            case INIDE: // in identifier
                if (!(isdigit(ch) || isalpha(ch))) {
                    unreadc();
                    save = FALSE;
                    state = DONE;
                    curr = MC_ID;
                }
                break;
            case INLES: // in less than
                state = DONE;
                if (ch == '=') {
                    curr = SS_LEQ;
                } else if (ch == '>') {
                    curr = SS_NEQ;
                } else {
                    unreadc();
                    save = FALSE;
                    curr = SS_LST;
                }
                break;
            case INCOM: // in comma
                state = DONE;
                if (ch == '=') {
                    curr = SS_ASGN;
                } else {
                    unreadc();
                    save = FALSE;
                    curr = SS_COLON;
                }
                break;
            case INGRE: // in great than
                state = DONE;
                if (ch == '=') {
                    curr = SS_GEQ;
                } else {
                    unreadc();
                    save = FALSE;
                    curr = SS_GTT;
                }
                break;
            case DONE:
            default:
                dbg("error state = %d", state);
                state = DONE;
                curr = ERROR;
                break;
        }

        // save ch to tokbuf[...]
        if ((save) && (i <= MAXTOKSIZE)) {
            tokbuf[i++] = (char) ch;
            tokbuf[i] = '\0';
        } else if (i > MAXTOKSIZE) {
            dbg("token size is too long, lineno = %d\n", lineno);
        }

        // post-processing works
        if (state == DONE) {
            tokbuf[i] = '\0';
            toklineno = lineno;
            if (curr == MC_ID) {
                curr = getkw(tokbuf);
            }
        }
    }

    dbg("token=%2d, buf=[%s], pos=%d:%d\n", curr, tokbuf, lineno, colmno);
    return curr;
}

// Source Code Line Buffer
char linebuf[MAXLINEBUF];
int bufsize = 0;
// when meet EOF, then set done to TRUE
static bool fileend = FALSE;

// hold file scan postion (line, column)
int lineno = 0;
int colmno = 0;

// read a character
static int readc(bool peek) {
    if (colmno < bufsize) {
        goto ready;
    }

    lineno++;
    if (fgets(linebuf, MAXLINEBUF - 1, source) == NULL) {
        fileend = TRUE;
        return EOF;
    }
    dbg("source L%03d: %s", lineno, linebuf);

    bufsize = strlen(linebuf);
    colmno = 0;
    goto ready;

ready:
    return (peek) ? linebuf[colmno] : linebuf[colmno++];
}

// unread a charachter
static void unreadc(void) {
    if (colmno <= 0) {
        panic("unread at line postion zero!");
    }
    if (!fileend) {
        colmno--;
    }
}

// Reserved Keyword Table
static struct _pl0e_keywords_struct {
    // keyword string
    char *str;
    // represented token
    token_t tok;
} PL0E_KEYWORDS[] = {
        { "array",     KW_ARRAY     }, //
        { "begin",     KW_BEGIN     }, //
        { "char",      KW_CHAR      }, //
        { "const",     KW_CONST     }, //
        { "do",        KW_DO        }, //
        { "downto",    KW_DOWNTO    }, //
        { "else",      KW_ELSE      }, //
        { "end",       KW_END       }, //
        { "for",       KW_FOR       }, //
        { "function",  KW_FUNCTION  }, //
        { "if",        KW_IF        }, //
        { "integer",   KW_INTEGER   }, //
        { "uinteger",  KW_UINTEGER  }, //
        { "of",        KW_OF        }, //
        { "procedure", KW_PROCEDURE }, //
        { "read",      KW_READ      }, //
        { "repeat",    KW_REPEAT    }, //
        { "then",      KW_THEN      }, //
        { "to",        KW_TO        }, //
        { "until",     KW_UNTIL     }, //
        { "var",       KW_VAR       }, //
        { "write",     KW_WRITE     }  //
};

#define MAXRESERVED (sizeof(PL0E_KEYWORDS) / sizeof(PL0E_KEYWORDS[0]))

// get keyword
// if s is keyword return token, otherwise return MC_ID
static token_t getkw(char *s) {
    int i;
    for (i = 0; i < MAXRESERVED; i++) {
        if (!strcmp(s, PL0E_KEYWORDS[i].str)) {
            return PL0E_KEYWORDS[i].tok;
        }
    }
    return MC_ID;
}
