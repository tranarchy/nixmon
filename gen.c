#include <time.h>
#include <stdio.h>
#include <sys/utsname.h>

#include "util/util.h"

int get_os(struct utsname utsname_buff) {
    pretty_print("OS", utsname_buff.sysname);

    return 0;
}

int get_kernel(struct utsname utsname_buff) {
    pretty_print("Kernel", utsname_buff.release);

    return 0;
}

int get_arch(struct utsname utsname_buff) {
    pretty_print("Arch", utsname_buff.machine);

    return 0;
}

int get_uptime(void) {
    struct timespec timespec_buff;
    char uptime_buff[32];

    #if defined(__linux__) || defined(__NetBSD__)
        int ret = clock_gettime(CLOCK_MONOTONIC, &timespec_buff);
    #else
        int ret = clock_gettime(CLOCK_UPTIME, &timespec_buff);
    #endif

    if (ret == -1) {
        return ret;
    }
    
    int uptime_sec = timespec_buff.tv_sec;
    int uptime_day = uptime_sec / 60 / 60 / 24;
    int uptime_hour = (uptime_sec / 60 / 60) - (uptime_day * 24);
    int uptime_min = (uptime_sec / 60) - (uptime_hour * 60) - (uptime_day * 24);

    snprintf(uptime_buff, 32, "%dd %dh %dm", uptime_day, uptime_hour, uptime_min);
    
    pretty_print("Uptime", uptime_buff);

    return 0;
}

int gen_init(void) {
    struct utsname utsname_buff;
    int ret = uname(&utsname_buff);

    if (ret == -1) {
        return ret;
    }

    pretty_print_title("gen");
    get_os(utsname_buff);
    get_arch(utsname_buff);
    get_kernel(utsname_buff);
    printf("\n");
    get_uptime();
    printf("\n");

    return 0;
}
