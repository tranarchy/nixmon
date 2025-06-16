#include <stdio.h>
#include <unistd.h>

#include <sys/sysctl.h>
#include <sys/vmmeter.h>

#ifdef __FreeBSD__
    #include <vm/vm_param.h>
#endif

#include "../include/info.h"
#include "../include/util.h"

int get_mem_usage(struct mem_info *mem) {
    int mib[2];

    size_t len;
    size_t slen;
    long long total;
    struct vmtotal vmtotal;

    mib[0] = CTL_HW;

    #ifdef __FreeBSD__
        mib[1] = HW_PHYSMEM;
    #else
        mib[1] = HW_PHYSMEM64;
    #endif

    len = sizeof(total);
    int ret = sysctl(mib, 2, &total, &len, NULL, 0);

    if (ret == -1) {
        return ret;
    }

    mib[0] = CTL_VM;
    mib[1] = VM_METER;

    slen = sizeof(vmtotal);
    ret = sysctl(mib, 2, &vmtotal, &slen, NULL, 0);

    if (ret == -1) {
        return ret;
    }

    long long pagesize = sysconf(_SC_PAGESIZE);

    #ifdef __OpenBSD__
        mem->used = get_mib(vmtotal.t_avm * pagesize);
    #else
        long long free = vmtotal.t_free * pagesize;
        mem->used = get_mib(total - free);       
    #endif


    mem->total = get_mib(total);

    if (mem->used > mem->used_max) {
        mem->used_max = mem->used;
    }

    return 0;
}
