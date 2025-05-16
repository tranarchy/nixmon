#include <stdio.h>
#include <sys/statvfs.h>

#include "util/util.h"

#if defined(__OpenBSD__)
    #define MOUNTPOINT "/home"
#else
    #define MOUNTPOINT "/"
#endif

void get_storage() {
    struct statvfs statvfs_buff;

    if (statvfs(MOUNTPOINT, &statvfs_buff) == -1) {
        return;
    }

    int total = get_gib(statvfs_buff.f_blocks * statvfs_buff.f_frsize);
    int free = get_gib(statvfs_buff.f_bfree * statvfs_buff.f_frsize);
    int used = total - free;

    int used_percent = 100.0 * used / total;

    print_progress("Storage (" MOUNTPOINT ")", used, total);
    printf(" (%dGiB / %dGiB) (%d%%)\n", used, total, used_percent);
}

void storage_init() {
    pretty_print_title("storage");
    get_storage();
    printf("\n");
}
