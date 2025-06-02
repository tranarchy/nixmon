#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>

#include "util/util.h"

#if defined(__linux__)
    #include <mntent.h>

#if defined(__NetBSD__)
    #include <sys/param.h>
#endif

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
    #include <sys/syslimits.h>
    #include <sys/mount.h>
#endif

struct storage_info {
    char mount_point[16];
    char size_suffix[4];

    int total;
    int used;
};

struct storage_info storage_info[32];

int storage_count;

int get_storage_size(char *mount_point, struct storage_info *storage_info) {
    struct statvfs statvfs_buff;

    if (statvfs(mount_point, &statvfs_buff) == -1) {
        return -1;
    }

    long long total = statvfs_buff.f_blocks * statvfs_buff.f_frsize;
    long long free = statvfs_buff.f_bfree * statvfs_buff.f_frsize;

    long long used = total - free;

    if (strlen(mount_point) >= 12) {
        strlcpy(storage_info->mount_point, mount_point, 12);
        strlcat(storage_info->mount_point, "...", 16);
    } else {
        strlcpy(storage_info->mount_point, mount_point, 16);
    }
   

    if (get_gib(total) == 0) {
        storage_info->used = get_mib(used);
        storage_info->total = get_mib(total);
        strlcpy(storage_info->size_suffix, "MiB", 4);
    } else {
        storage_info->used = get_gib(used);
        storage_info->total = get_gib(total);
        strlcpy(storage_info->size_suffix, "GiB", 4);
    }

    return 0;
}


void get_storages(struct storage_info *storage_info) {

    storage_count = 0;

    #if defined(__linux__)

        FILE *fp;
        struct mntent *mntent;

        char *allowed_filesystems[] = { "ext2", "ext3", "ext4", "jfs", "xfs", "vfat", "exfat", "ntfs", "ntfs-3g", "btrfs", "zfs", "iso9660" };

        fp = fopen("/etc/mtab", "r");

        while ((mntent = getmntent(fp)) != NULL) {
            int ret = -1;

            for (long unsigned int i = 0; i < sizeof(allowed_filesystems) / sizeof(allowed_filesystems[0]); i++) {
                if (strcmp(allowed_filesystems[i], mntent->mnt_type) == 0) {
                    ret = 0;
                    break;
                }
            }

            if (ret == -1) {
                continue;
            }

            ret = get_storage_size(mntent->mnt_dir, &storage_info[storage_count]);
           
            if (ret == -1) {
                continue;
            }
           
            storage_count++;
        
        }
        fclose(fp);
    
    #elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)


        #if defined(__NetBSD__)
            struct statvfs *statfs;
        #else
            struct statfs *statfs;
        #endif
    
        storage_count = getmntinfo(&statfs, MNT_WAIT);

        if (storage_count == 0) {
            return;
        }

        #if defined(__FreeBSD__)
            char *mnt_blacklist[] = { "/dev", "/home", "/zroot", "/usr", "/tmp", "/var" };
        #endif

        for (int i = 0; i < storage_count; i++) {
            #if defined(__FreeBSD__)
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

            get_storage_size(statfs[i].f_mntonname, &storage_info[i]);

        }
    #endif
}

void storage_init(void) {
   
    get_storages(storage_info);
    
    print_title("storage");
    for (int i = 0; i < storage_count; i++) {
        struct storage_info storage;

        storage = storage_info[i];

        int used_percent = 100.0 * storage.used / storage.total;

        print_progress(storage.mount_point, storage.used, storage.total);
        printf(" (%d%s / %d%s) (%d%%)\n", storage.used, storage.size_suffix, storage.total, storage.size_suffix, used_percent);
    }

    printf("\n");
    
}
