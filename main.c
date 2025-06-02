#if !(defined(__linux__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__))
    #warning "Unsupported OS!"
#endif

#include <stdio.h>
#include <unistd.h>
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
    printf(SHOW_CURSOR);
    exit(0);
}

int main(void) {
    tcgetattr(0, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new);

    printf(HIDE_CURSOR);

    signal(SIGINT, INT_handler);
    
    while (1) {
        printf(CLEAR);

        draw_box(1);

        gen_init();
        cpu_init();
        mem_init();
        storage_init();

        #if !(defined(__APPLE__))
            gpu_init();
        #endif
    
        draw_box(0);

        sleep(1);
    }

    return 0;
}
