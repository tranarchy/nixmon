#include <string.h>
#include <sys/statvfs.h>

#include "../include/util.h"
#include "../include/info.h"

int get_storage_size(char *mount_point, struct storage_info *storages) {
    struct statvfs statvfs_buff;

    if (statvfs(mount_point, &statvfs_buff) == -1) {
        return -1;
    }

    long long total = statvfs_buff.f_blocks * statvfs_buff.f_frsize;
    long long free = statvfs_buff.f_bfree * statvfs_buff.f_frsize;

    long long used = total - free;

    if (strlen(mount_point) >= 12) {
        strlcpy(storages->mount_point, mount_point, 12);
        strlcat(storages->mount_point, "...", 16);
    } else {
        strlcpy(storages->mount_point, mount_point, 16);
    }

    if (get_gib(total) == 0) {
        storages->used = get_mib(used);
        storages->total = get_mib(total);
        strlcpy(storages->size_suffix, "MiB", 4);
    } else {
        storages->used = get_gib(used);
        storages->total = get_gib(total);
        strlcpy(storages->size_suffix, "GiB", 4);
    }

    return 0;
}
