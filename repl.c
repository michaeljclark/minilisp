// This software is in the public domain.

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>

#include <locale.h>
#include <signal.h>

#include "minilisp.h"
#include "histedit.h"

#ifdef WIN32
#include <windows.h>
extern int _isatty(int);
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

static char* prompt(EditLine *el) {
    return "> ";
}

volatile sig_atomic_t sig = 0;

static void sig_handler(int i) {
    sig = i;
}

int main(int argc, char **argv) {
    EditLine *el = NULL;
    const char *buf;
    Tokenizer *tok;
    History *hist;
    HistEvent ev;
    int num, ncont;
    int ac, cc, co;
    const char **av;
    const LineInfo *li;

    void *roots[3] = { NULL, NULL, ((void *)-1) };
    void *root = roots;
    Obj **expr = (Obj **)(roots + 0);
    Obj **env = (Obj **)(roots + 1);
    init(&root, env);

    if (isatty(fileno(stdout))) {

        setlocale(LC_CTYPE, "");

        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        hist = history_init();
        history(hist, &ev, H_SETSIZE, 100);

        tok  = tok_init(NULL);
        el = el_init(*argv, stdin, stdout, stderr);

        el_set(el, EL_EDITOR, "emacs");
        el_set(el, EL_SIGNAL, 1);
        el_set(el, EL_PROMPT_ESC, prompt, '\1');
        el_set(el, EL_HIST, history, hist);

        el_source(el, NULL);

        for (;;)
        {
            buf = el_gets(el, &num);
            li = el_line(el);

            if (sig) {
                fprintf(stderr, "signal %d\n", (int)sig);
                sig = 0;
                continue;
            }

            if (buf == NULL) {
                return 0;
            }

            if (num == 1) {
                continue;
            }

            ac = cc = co = 0;
            if ((ncont = tok_line(tok, li, &ac, &av, &cc, &co)) < 0) {
                error("Internal error");
            }

            history(hist, &ev, H_ENTER, buf);

            if (ncont > 0 || el_parse(el, ac, av) == -1) {
                buffer_expr(li->buffer, strlen(li->buffer));
                Obj* result = read_eval_expr(root, env, expr);
                if (result) {
                    print(result);
                    printf("\n");
                }
            }

            tok_reset(tok);
        }

        el_end(el);
        tok_end(tok);
        history_end(hist);
    }
    else {
        // The main loop
        for (;;) {
            Obj* result = read_eval_expr(root, env, expr);
            if (!result)
                return 0;
            if (result == Err)
                exit(1);
            print(result);
            printf("\n");
        }
    }

    return (0);
}
