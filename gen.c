#include <time.h>
#include <stdio.h>
#include <sys/utsname.h>

#include "info.h"

#include "util/util.h"

int get_uptime(struct gen_info *gen) {
    struct timespec timespec_buff;
   

    #if defined(__linux__) || defined(__NetBSD__) || defined(__APPLE__)
        int ret = clock_gettime(CLOCK_MONOTONIC, &timespec_buff);
    #else
        int ret = clock_gettime(CLOCK_UPTIME, &timespec_buff);
    #endif

    if (ret == -1) {
        return ret;
    }
    
    int uptime_sec = timespec_buff.tv_sec;

    gen->uptime_day = uptime_sec / 60 / 60 / 24;
    gen->uptime_hour = uptime_sec / 60 / 60 % 24;
    gen->uptime_min = uptime_sec / 60 % 60;
    gen->uptime_sec = uptime_sec % 60;

    return 0;
}

void gen_init(struct gen_info *gen) {
    struct utsname utsname_buff;

    print_title("gen");
    if (uname(&utsname_buff) != -1) {
        pretty_print("OS", utsname_buff.sysname);
        pretty_print("Kernel", utsname_buff.release);
        pretty_print("Arch", utsname_buff.machine);
        printf("\n");
    }

    if (get_uptime(gen) != -1) {
        char uptime_buff[32];
        snprintf(uptime_buff, 32, "%dd %dh %dm %ds", 
            gen->uptime_day, 
            gen->uptime_hour, 
            gen->uptime_min,
            gen->uptime_sec
        );
        pretty_print("Uptime", uptime_buff);
        printf("\n");
    }
}
