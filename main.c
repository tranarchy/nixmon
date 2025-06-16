#if !(defined(__linux__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__))
    #warning "Unsupported OS!"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>

#include "info.h"
#include "main.h"

#include "util/util.h"
#include "util/ansi.h"

struct termios old, new;

void int_handler(int sig) {
    signal(sig, SIG_IGN);
    tcsetattr(0, TCSANOW, &old);
    printf(CLEAR);
    printf(SHOW_CURSOR);
    exit(0);
}

int main(void) {
    struct gen_info gen = { 0 };
    struct cpu_info cpu = { 0 };
    struct mem_info mem = { 0 };
    struct storage_info storages[32];
    struct gpu_info gpu = { 0 };

    tcgetattr(0, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new);

    printf(HIDE_CURSOR);

    signal(SIGINT, int_handler);
    
    for (;;) {
        printf(CLEAR);

        draw_box(1);

        gen_init(&gen);
        cpu_init(&cpu);
        mem_init(&mem);
        storage_init(storages);

        #if !(defined(__APPLE__))
            gpu_init(&gpu);
        #endif
    
        draw_box(0);

        sleep(1);
    }

    return 0;
}
