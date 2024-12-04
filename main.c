#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

#define DIGITS 10
#define STDIN_FILENO 0

enum sig_state {
    keyed_interrupt = 0,
    keep_going = 1,
};

struct content_stats {
    int nwhite;
    int ntab;
    int nspace;
    int nlines;
    int nother;
    int ndigit[DIGITS];
};

struct content_stats new_stats() {
    struct content_stats s = {0,0};

    int i;
    for (i = 0; i < DIGITS; ++i) { s.ndigit[i] = 0; }

    return s;
}

void inc_other(struct content_stats *s) {
    s->nother++;
}

void inc_whitespace(struct content_stats *s) {
    s->nwhite++;
}
void inc_digit(struct content_stats *s, char c) {
    s->ndigit[c-'0']++;
}

void keyboard_interrupt(int);
void nonfile_routine();
void file_routine(FILE *f);

enum sig_state sig = keep_going;

int index_argv(char *term, int argc, char** argv) {
    int i;
    for (i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], term)) continue;
        return i;
    }

    return -1;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        return 0;
    }

    int ret = index_argv("-p", argc, argv);
    if (ret != -1) {
        if ((ret+1) > argc-1) {
            printf("contents: no path to file");
            return 1;
        }
    }

    --argc;
    printf("%s\n", argv[argc]);

    struct stat s;
    ret = stat(argv[argc], &s);

    if (ret == -1) {
        printf("content-data: cannot open file\n");
        return 1;
    }

    FILE *fptr = fopen(argv[argc], "r");
    if (fptr == NULL) {
        printf("content-data: cannot open file\n");
        return 1;
    }

    file_routine(fptr);

    return 0;
}

void keyboard_interrupt(int s) {
    extern enum sig_state sig;
    sig = keyed_interrupt;
}

void file_routine(FILE *f) {
    int i, c;
    extern enum sig_state sig;
    struct content_stats s = new_stats();

    do {
        if (c >= '0' && c <= '9') {
            inc_digit(&s, c);
        }

        if (c == '\t' || c == ' ') {
            inc_whitespace(&s);
            continue;
        }

        inc_other(&s);
    } while ((c = fgetc(f)) != EOF);

    printf("digits: \n");
    for (i = 0; i < 10; ++i) {
        printf("\t%d\t  ->  %d\n", i, s.ndigit[i]);
    }

    printf("\nwhitespace: %d\n", s.nwhite);
    printf("other: %d\n", s.nother);
}

void nonfile_routine() {
    int i, c;
    extern enum sig_state sig;
    struct content_stats s = new_stats();

    // catch keyboard interrupt;
    signal(SIGINT, keyboard_interrupt);

    do {
        if (sig == keyed_interrupt) {
            break;
        }

        if (c >= '0' && c <= '9') {
            inc_digit(&s, c);
        }

        if (c == '\t' || c == ' ') {
            inc_whitespace(&s);
            continue;
        }

        inc_other(&s);
    } while ((c = getchar()) != '\v');

    printf("digits: \n");
    for (i = 0; i < 10; ++i) {
        printf("\t%d\t  ->  %d\n", i, s.ndigit[i]);
    }

    printf("\nwhitespace: %d\n", s.nwhite);
    printf("other: %d\n", s.nother);
}
