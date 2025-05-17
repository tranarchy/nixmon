#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>

#include "util/util.h"

#if defined(__linux__)
    #include <mntent.h>

#if defined(__NetBSD__)
    #include <sys/param.h>
#endif

#elif defined(__FreeBSD__) || defined(__NetBSD__)
    #include <sys/mount.h>
#endif

void get_storage() {

    struct statvfs statvfs_buff;

    #if defined(__linux__)

        FILE *fp;
        struct mntent *mntent;

        char *allowed_filesystems[] = { "ext2", "ext3", "ext4", "jfs", "xfs", "vfat", "exfat", "ntfs", "ntfs-3g", "btrfs", "zfs", "iso9660" };

        fp = fopen("/etc/mtab", "r");
        while ((mntent = getmntent(fp)) != NULL) {
            int ret = -1;

            for (int i = 0; i < sizeof(allowed_filesystems) / sizeof(allowed_filesystems[0]); i++) {
                if (strcmp(allowed_filesystems[i], mntent->mnt_type) == 0) {
                    ret = 0;
                    break;
                }
            }

            if (ret == -1) {
                continue;
            }

            if (statvfs(mntent->mnt_dir, &statvfs_buff) == -1) {
                return;
            }
        
            long long total = statvfs_buff.f_blocks * statvfs_buff.f_frsize;
            long long free = statvfs_buff.f_bfree * statvfs_buff.f_frsize;
        
            long long used = total - free;

            int used_percent = 100.0 * used / total;
        
            print_progress(mntent->mnt_dir, used, total);

            if (get_gib(total) == 0) {
                printf(" (%dMiB / %dMiB) (%d%%)\n", get_mib(used), get_mib(total), used_percent);
            } else {
                printf(" (%dGiB / %dGiB) (%d%%)\n", get_gib(used), get_gib(total), used_percent);
            }
        
        }
        fclose(fp);
    
    #elif defined(__FreeBSD__) || defined(__NetBSD__)


        #if defined(__NetBSD__)
            struct statvfs *statfs;
        #else
            struct statfs *statfs;
        #endif
    
        int count = getmntinfo(&statfs, MNT_WAIT);

        if (count == 0) {
            return;
        }

        #if defined(__FreeBSD__)
            char *mnt_blacklist[] = { "/dev", "/home", "/zroot", "/usr", "/tmp", "/var" };
        #endif

        for (int i = 0; i < count; i++) {
            #if defined(__FreeBSD__)
                int ret = 0;

                for (int j = 0; j < sizeof(mnt_blacklist) / sizeof(mnt_blacklist[0]); j++) {
                    if (strstr(statfs[i].f_mntonname, mnt_blacklist[j]) != NULL) {
                        ret = -1;
                    }
                }

                if (ret == -1) {
                    continue;
                }
            #endif

            if (statvfs(statfs[i].f_mntonname, &statvfs_buff) == -1) {
                return;
            }
        
            long long total = statvfs_buff.f_blocks * statvfs_buff.f_frsize;
            long long free = statvfs_buff.f_bfree * statvfs_buff.f_frsize;
        
            long long used = total - free;

            int used_percent = 100.0 * used / total;
        
            print_progress(statfs[i].f_mntonname, used, total);

            if (get_gib(total) == 0) {
                printf(" (%dMiB / %dMiB) (%d%%)\n", get_mib(used), get_mib(total), used_percent);
            } else {
                printf(" (%dGiB / %dGiB) (%d%%)\n", get_gib(used), get_gib(total), used_percent);
            }
        }
    #endif
}

void storage_init() {
    pretty_print_title("storage");
    get_storage();
    printf("\n");
}
