#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>

#include "main.h"
#include "util/util.h"
#include "util/ansi.h"

struct termios old, new;

void INT_handler(int sig) {
    signal(sig, SIG_IGN);
    tcsetattr(0, TCSANOW, &old);
    printf(CLEAR);
    exit(0);
}

int main() {
    tcgetattr(0, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new);

    signal(SIGINT, INT_handler);

    struct timeval tv;
    
    while (1) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        printf(CLEAR);

        draw_box(1);

        gen_init();
        cpu_init();
        mem_init();
        storage_init();

        #if defined(__linux__)
            amd_gpu_init();
            i915_init();
        #endif
    
        draw_box(0);

        select(0, NULL, NULL, NULL, &tv);     
    }

    return 0;
}
