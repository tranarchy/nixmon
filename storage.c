#include <stdio.h>
#include <sys/statvfs.h>

#include "util/util.h"

void get_storage() {
    struct statvfs statvfs_buff;

    if (statvfs("/", &statvfs_buff) == -1) {
        return;
    }

    int total = get_gib(statvfs_buff.f_blocks * statvfs_buff.f_frsize);
    int free = get_gib(statvfs_buff.f_bfree * statvfs_buff.f_frsize);
    int used = total - free;

    print_progress("Storage (/)", used, total);

    printf(" (%dGiB / %dGiB)\n", used, total);
}

void storage_init() {
    pretty_print_title("storage");
    get_storage();
    printf("\n");
}