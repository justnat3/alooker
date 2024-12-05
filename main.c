#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIGITS 10
#define STDIN_FILENO 0

enum sig_state {
    KeyedInterrupt = 0,
    KeepGoing = 1,
};

enum OP {
    Manual,
    FIO,
};

// content statistics
struct cstats {
    int nwhite;
    int ntab;
    int nspace;
    int nlines;
    int nother;
    int ndigit[DIGITS];
};

struct cstats new_stats() {
    struct cstats s = {0,0};

    int i;
    for (i = 0; i < DIGITS; ++i) { s.ndigit[i] = 0; }

    return s;
}

void inc_other(struct cstats *s) {
    s->nother++;
}

void inc_whitespace(struct cstats *s) {
    s->nwhite++;
}
void inc_digit(struct cstats *s, char c) {
    s->ndigit[c-'0']++;
}

void keyboard_interrupt(int);
void nonfile_routine();
void file_routine(FILE *f);

enum sig_state sig = KeepGoing;

int index_argv(char *term, int argc, char** argv) {
    int i;
    for (i = 0; i < argc; ++i) {
        if (strcmp(argv[i], term) != 0) continue;
        return i;
    }

    return -1;
}
void print_help() {
    printf("OPTIONS: [FILE|PIPE|STDIN]\n");
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        if (isatty(fileno(stdin)) == 0) {
            file_routine(stdin);
            return 0;
        }
        return 0;
    }

    int ret = index_argv("-h", argc, argv);
    if (ret != -1) {
        print_help();
        return 0;
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
    sig = KeyedInterrupt;
}
int fetch_char(enum OP o, FILE* f) {
    int c;
    if (o == Manual ) {
        c = getchar();
    } else {
        if (f == NULL){ return EOF; }
        c = fgetc(f);
    }

    return c;
}

void fill_stats(enum OP o, FILE *f, struct cstats*s) {
    int i, c;

    do {
        if (c >= '0' && c <= '9') {
            inc_digit(s, c);
        }

        if (c == '\t' || c == ' ') {
            inc_whitespace(s);
            continue;
        }

        inc_other(s);
    } while ((c = fetch_char(o, f)) != EOF);
}

void file_routine(FILE *f) {
    extern enum sig_state sig;
    struct cstats s = new_stats();
    fill_stats(FIO, f, &s);

    printf("digits: \n");
    for (int i = 0; i < 10; ++i) {
        printf("\t%d\t  ->  %d\n", i, s.ndigit[i]);
    }

    printf("\nwhitespace: %d\n", s.nwhite);
    printf("other: %d\n", s.nother);
    fflush(stdout);
}

void nonfile_routine() {
    int i, c;
    extern enum sig_state sig;
    struct cstats s = new_stats();
    signal(SIGINT, keyboard_interrupt);
    fill_stats(FIO, NULL, &s);
    // catch keyboard interrupt;

    printf("digits: \n");
    for (i = 0; i < 10; ++i) {
        printf("\t%d\t  ->  %d\n", i, s.ndigit[i]);
    }

    printf("\nwhitespace: %d\n", s.nwhite);
    printf("other: %d\n", s.nother);
}
