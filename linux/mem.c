#include <stdio.h>
#include <string.h>

#include "../include/info.h"

int get_mem_usage(struct mem_info *mem) {

    int available;

    FILE *fp;
    char line[128];

    fp = fopen("/proc/meminfo", "r");

    if (fp == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "MemTotal:") != NULL) {
            if (sscanf(line, "MemTotal: %d kB", &mem->total) == 1) {
                continue;
            }
        }

        if (strstr(line, "MemAvailable:") != NULL) {
            if (sscanf(line, "MemAvailable: %d kB", &available) == 1) {
                continue;
            }
        }
    }

    fclose(fp);

    mem->used = mem->total - available;

    mem->total /= 1024;
    mem->used /= 1024;

    if (mem->used > mem->used_max) {
        mem->used_max = mem->used;
    }

    return 0;
}
