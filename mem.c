#include <stdio.h>
#include <string.h>

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #include <unistd.h>

    #include <sys/sysctl.h>
    #include <sys/vmmeter.h>
#endif

#if defined(__FreeBSD__)
    #include <vm/vm_param.h>
#endif

#include "util/util.h"

int mem_total, mem_used_max = 0;

void get_mem_usage() {
    int used = 0;

    #if defined(__linux__)

        FILE *fp;
        char line[128];

        int available = 0;

        fp = fopen("/proc/meminfo", "r");

        if (fp == NULL) {
            return;
        }

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "MemTotal:") != NULL) {
                if (sscanf(line, "MemTotal: %d kB", &mem_total) == 1) {
                    continue;
                }
            }

            if (strstr(line, "MemAvailable:") != NULL) {
                if (sscanf(line, "MemAvailable: %d kB", &available) == 1) {
                    continue;
                }
            }

            if (mem_total > 0 && available > 0) {
                break;
            }
        }

        fclose(fp);

        used = mem_total - available;

        mem_total /= 1024;
        used /= 1024;

    #elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
        
        int mib[2];
        size_t len, slen;
        long long total;

        struct vmtotal vmtotal;

        mib[0] = CTL_HW;
        #if defined(__FreeBSD__)
            mib[1] = HW_PHYSMEM;
        #else
            mib[1] = HW_PHYSMEM64;
        #endif

        slen = sizeof(total);
        int ret = sysctl(mib, 2, &total, &slen, NULL, 0);

        if (ret == -1) {
            return;
        }

        mib[0] = CTL_VM;
        mib[1] = VM_METER;

        len = sizeof(struct vmtotal);
        ret = sysctl(mib, 2, &vmtotal, &len, NULL, 0);

        if (ret == -1) {
            return;
        }

        long long pagesize = sysconf(_SC_PAGESIZE);

        #if defined(__OpenBSD__)
            long long active = vmtotal.t_avm * pagesize;
            used = get_mib(active);
        #else
            long long free = vmtotal.t_free * pagesize;
            used = get_mib(total - free);
        #endif

        mem_total = get_mib(total);

    #endif

    if (used > mem_used_max) {
        mem_used_max = used;
    }

    print_progress("RAM usage", used, mem_total);

    printf(" (%dMiB / %dMiB)\n", used, mem_total);
}

void get_mem_usage_max() {
    print_progress("Max RAM usage", mem_used_max, mem_total);

    printf(" (%dMiB / %dMiB)\n", mem_used_max, mem_total);
}

void mem_init() {
    pretty_print_title("mem");
    get_mem_usage();
    get_mem_usage_max();
    printf("\n");
}
