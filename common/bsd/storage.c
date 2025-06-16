#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>

#include <sys/mount.h>
#include <sys/syslimits.h>

#ifdef __NetBSD__
    #include <sys/param.h>
#endif

#include "../../include/info.h"
#include "../../include/storage.h"


void get_storages(struct storage_info *storages) {
    int storage_count = 0;

    #ifdef __NetBSD__
        struct statvfs *statfs;
    #else
        struct statfs *statfs;
    #endif
    
    storage_count = getmntinfo(&statfs, MNT_WAIT);

    if (storage_count == 0) {
        return;
    }

    #ifdef __FreeBSD__
        char *mnt_blacklist[] = { "/dev", "/home", "/zroot", "/usr", "/tmp", "/var" };
    #endif

    for (int i = 0; i < 32; i++) {
        #ifdef __FreeBSD__
            int ret = 0;

            for (long unsigned int j = 0; j < sizeof(mnt_blacklist) / sizeof(mnt_blacklist[0]); j++) {
                if (strstr(statfs[i].f_mntonname, mnt_blacklist[j]) != NULL) {
                    ret = -1;
                }
            }

            if (ret == -1) {
                continue;
            }
        #endif

        get_storage_size(statfs[i].f_mntonname, &storages[i]);

    }
}
