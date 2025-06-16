#include <stdio.h>
#include <string.h>

#include "info.h"

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #include <unistd.h>

    #include <sys/sysctl.h>
    #include <sys/vmmeter.h>
#endif

#if defined(__APPLE__)
    #include <unistd.h>

    #include <mach/mach.h>    
    #include <sys/sysctl.h>
    
#endif

#if defined(__FreeBSD__)
    #include <vm/vm_param.h>
#endif

#include "util/util.h"

int get_mem_usage(struct mem_info *mem) {

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

    #elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
        
        int mib[2];
        size_t slen;
        long long total;

        #if defined(__APPLE__)
            mach_msg_type_number_t len = HOST_VM_INFO64_COUNT;

            vm_statistics64_data_t vm_stats;            
        #else
            size_t len;

            struct vmtotal vmtotal;
        #endif

        mib[0] = CTL_HW;
        #if defined(__FreeBSD__) || defined(__APPLE__)
            mib[1] = HW_PHYSMEM;
        #else
            mib[1] = HW_PHYSMEM64;
        #endif

        slen = sizeof(total);
        int ret = sysctl(mib, 2, &total, &slen, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        #if defined(__APPLE__)
            ret = host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stats, &len);

            if (ret == -1) {
                return ret;
            }
        #else
            mib[0] = CTL_VM;
            mib[1] = VM_METER;

            len = sizeof(vmtotal);
            ret = sysctl(mib, 2, &vmtotal, &len, NULL, 0);

            if (ret == -1) {
                return ret;
            }
        #endif

        long long pagesize = sysconf(_SC_PAGESIZE);

        #if defined(__OpenBSD__)
            long long active = vmtotal.t_avm * pagesize;
            mem->used = get_mib(active);
        #elif defined(__APPLE__)
            long long active = vm_stats.active_count * pagesize;
            long long wire = vm_stats.wire_count * pagesize;
            long long compression = vm_stats.compressor_page_count * pagesize;
            mem->used = get_mib(active + wire + compression);
        #else
            long long free = vmtotal.t_free * pagesize;
            mem->used = get_mib(total - free);
        #endif

        mem->total = get_mib(total);

    #endif

    if (mem->used > mem->used_max) {
        mem->used_max = mem->used;
    }

    return 0;
}

void mem_init(struct mem_info *mem) {

    if (get_mem_usage(mem) != -1) {
        print_title("mem");
        print_progress("RAM usage", mem->used, mem->total);
        printf(" (%dMiB / %dMiB)\n",  mem->used, mem->total);
    
        print_progress("Max RAM usage", mem->used_max, mem->total);
        printf(" (%dMiB / %dMiB)\n",  mem->used_max, mem->total);
        printf("\n");
    }
}
