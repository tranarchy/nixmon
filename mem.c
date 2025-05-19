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

struct mem_info {
    int total;

    int used;
    int used_max;
};

struct mem_info mem_info;

int get_mem_usage(struct mem_info *mem_info) {

    #if defined(__linux__)

        int available;

        FILE *fp;
        char line[128];

        fp = fopen("/proc/meminfo", "r");

        if (fp == NULL) {
            return -1;
        }

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "MemTotal:") != NULL) {
                if (sscanf(line, "MemTotal: %d kB", &mem_info->total) == 1) {
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

        mem_info->used = mem_info->total - available;

        mem_info->total /= 1024;
        mem_info->used /= 1024;

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
            return ret;
        }

        mib[0] = CTL_VM;
        mib[1] = VM_METER;

        len = sizeof(struct vmtotal);
        ret = sysctl(mib, 2, &vmtotal, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        long long pagesize = sysconf(_SC_PAGESIZE);

        #if defined(__OpenBSD__)
            long long active = vmtotal.t_avm * pagesize;
            mem_info->used = get_mib(active);
        #else
            long long free = vmtotal.t_free * pagesize;
            mem_info->used = get_mib(total - free);
        #endif

        mem_info->total = get_mib(total);

    #endif

    if (mem_info->used > mem_info->used_max) {
        mem_info->used_max = mem_info->used;
    }

    return 0;
}

void mem_init(void) {

    if (get_mem_usage(&mem_info) != -1) {
        print_title("mem");
        print_progress("RAM usage", mem_info.used, mem_info.total);
        printf(" (%dMiB / %dMiB)\n",  mem_info.used, mem_info.total);
    
        print_progress("Max RAM usage", mem_info.used_max, mem_info.total);
        printf(" (%dMiB / %dMiB)\n",  mem_info.used_max, mem_info.total);
        printf("\n");
    }
}
