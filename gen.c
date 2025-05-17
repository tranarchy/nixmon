#include <time.h>
#include <stdio.h>
#include <sys/utsname.h>

#include "util/util.h"

void get_os(struct utsname utsname_buff) {
    pretty_print("OS", utsname_buff.sysname);
}

void get_kernel(struct utsname utsname_buff) {
    pretty_print("Kernel", utsname_buff.release);
}

void get_arch(struct utsname utsname_buff) {
    pretty_print("Arch", utsname_buff.machine);
}

void get_uptime() {
    struct timespec timespec_buff;
    char uptime_buff[32];

    int ret = 0;

    #if defined(__linux__) || defined(__NetBSD__)
        ret = clock_gettime(CLOCK_MONOTONIC, &timespec_buff);
    #else
        ret = clock_gettime(CLOCK_UPTIME, &timespec_buff);
    #endif

    if (ret == -1) {
        return;
    }
    
    int uptime_sec = timespec_buff.tv_sec;
    int uptime_day = uptime_sec / 60 / 60 / 24;
    int uptime_hour = (uptime_sec / 60 / 60) - (uptime_day * 24);
    int uptime_min = (uptime_sec / 60) - (uptime_hour * 60) - (uptime_day * 24);

    snprintf(uptime_buff, 32, "%dd %dh %dm", uptime_day, uptime_hour, uptime_min);
    
    pretty_print("Uptime", uptime_buff);
}

void gen_init() {
    struct utsname utsname_buff;
    uname(&utsname_buff);

    pretty_print_title("gen");
    get_os(utsname_buff);
    get_arch(utsname_buff);
    get_kernel(utsname_buff);
    printf("\n");
    get_uptime();
    printf("\n");
}
