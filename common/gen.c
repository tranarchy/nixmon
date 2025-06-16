#include <time.h>

#include "../include/info.h"

int get_uptime(struct gen_info *gen) {
    struct timespec timespec_buff;
   
    #if defined(__linux__) || defined(__NetBSD__) || defined(__APPLE__)
        int clock_id = CLOCK_MONOTONIC;
    #else
        int clock_id = CLOCK_UPTIME;
    #endif

    int ret = clock_gettime(clock_id, &timespec_buff);

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
