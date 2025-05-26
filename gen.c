#include <time.h>
#include <stdio.h>
#include <sys/utsname.h>

#include "util/util.h"

struct gen_info {
    int uptime_day;
    int uptime_hour;
    int uptime_min;
};

struct gen_info gen_info;

int get_uptime(struct gen_info *gen_info) {
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

    gen_info->uptime_day = uptime_sec / 60 / 60 / 24;
    gen_info->uptime_hour = (uptime_sec / 60 / 60) - ( gen_info->uptime_day * 24);
    gen_info->uptime_min = (uptime_sec / 60) - ( gen_info->uptime_hour * 60) - ( gen_info->uptime_day * 24);

    return 0;
}

void gen_init(void) {
    struct utsname utsname_buff;

    print_title("gen");
    if (uname(&utsname_buff) != -1) {
        pretty_print("OS", utsname_buff.sysname);
        pretty_print("Kernel", utsname_buff.release);
        pretty_print("Arch", utsname_buff.machine);
        printf("\n");
    }

    if (get_uptime(&gen_info) != -1) {
        char uptime_buff[32];
        snprintf(uptime_buff, 32, "%dd %dh %dm", gen_info.uptime_day, gen_info.uptime_hour, gen_info.uptime_min);
        pretty_print("Uptime", uptime_buff);
        printf("\n");
    }
}
