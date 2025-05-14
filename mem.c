#include <stdio.h>
#include <string.h>

#include "util/util.h"

long int mem_total, mem_max_used = 0;

void get_mem_usage() {
    FILE *fp;
    char line[128];
    long int available, used = 0;

    fp = fopen("/proc/meminfo", "r");

    if (fp == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "MemTotal:") != NULL) {
            if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) {
                continue;
            }
        }

        if (strstr(line, "MemAvailable:") != NULL) {
            if (sscanf(line, "MemAvailable: %ld kB", &available) == 1) {
                continue;
            }
        }
    }

    fclose(fp);

    used = mem_total - available;

    mem_total /= 1024;
    used /= 1024;

    if (used > mem_max_used) {
        mem_max_used = used;
    }

    print_progress("RAM usage", used, mem_total);

    printf(" (%ldMiB / %ldMiB)\n", used, mem_total);
}

void get_mem_usage_max() {
    print_progress("Max RAM usage", mem_max_used, mem_total);

    printf(" (%ldMiB / %ldMiB)\n", mem_max_used, mem_total);
}

void mem_init() {
    pretty_print_title("mem");
    get_mem_usage();
    get_mem_usage_max();
    printf("\n");
}