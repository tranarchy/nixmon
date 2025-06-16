
#include <stdio.h>
#include <mntent.h>
#include <string.h>

#include "../include/info.h"
#include "../include/storage.h"

void get_storages(struct storage_info *storages) {
    FILE *fp;
    struct mntent *mntent;

    char *allowed_filesystems[] = { "ext2", "ext3", "ext4", "jfs", "xfs", "vfat", "exfat", "ntfs", "ntfs-3g", "btrfs", "zfs", "iso9660" };

    int storage_count = 0;

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

        ret = get_storage_size(mntent->mnt_dir, &storages[storage_count]);
           
        if (ret == -1) {
            continue;
        }
           
        storage_count++;
        
    }
    
    fclose(fp);
}
