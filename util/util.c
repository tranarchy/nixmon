#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>

#include "../include/ansi.h"

int get_gib(long long value) {
    return value / (1024.0 * 1024.0 * 1024.0);
}

int get_mib(long long value) {
    return value / (1024.0 * 1024.0);
}   

void draw_box(int top) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);


    printf(top ? "%s%s┏" : "%s%s┗", RESET, FG);

    int cols = w.ws_col - 2 - (top ? 6 : 0);

    for (int i = 0; i < cols; i++) {
        if (i == (w.ws_col / 2) - 4 && top) {
            printf("nixmon");
        }
        printf("━");
    }

    printf(top ? "┓\n\n%s" : "┛\n\n%s", RESET);
}

void pretty_print(char *content, char *content2) {

    printf("\t%s%s %s", FG, content, RESET);

    if (strlen(content) < 7) {
        printf("\t");
    }

    printf("\t%s\n", content2);
}

void print_title(char *content) {
    printf("\t\b\b%s%s\n\n%s", BG, content, RESET);
}

void print_progress(char *name, float value, float max_value) {
    int percent = (value / max_value) * 100;

    printf("\t%s%s %s", FG, name, RESET);


    if (strlen(name) < 7) {
        printf("\t");
    }

    printf("\t[");
    for (int i = 0; i < 50; i++) {
        if (percent / 2 > i) {
            printf("%s %s", BG, RESET);
        } else {
            printf(" ");
        }
    }
    printf("]");
}
