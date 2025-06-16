#include <stdio.h>
#include <unistd.h>

#include <mach/mach.h>

#include <sys/sysctl.h>

#include "../../include/info.h"
#include "../../include/util.h"

int get_mem_usage(struct mem_info *mem) {
    int mib[2];

    size_t len;
    size_t slen;
    long long total;

    vm_statistics64_data_t vm_stats; 

    mach_msg_type_number_t len = HOST_VM_INFO64_COUNT;

    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;

    len = sizeof(total);
    int ret = sysctl(mib, 2, &total, &len, NULL, 0);

    if (ret == -1) {
        return ret;
    }

    ret = host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stats, &len);

    if (ret == -1) {
        return ret;
    }

    long long pagesize = sysconf(_SC_PAGESIZE);

    long long active = vm_stats.active_count * pagesize;
    long long wire = vm_stats.wire_count * pagesize;
    long long compression = vm_stats.compressor_page_count * pagesize;
    mem->used = get_mib(active + wire + compression);

    mem->total = get_mib(total);

    if (mem->used > mem->used_max) {
        mem->used_max = mem->used;
    }

    return 0;
}
