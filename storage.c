#include <stdio.h>

#include "include/util.h"
#include "include/info.h"
#include "include/storage.h"

void storage_init(struct storage_info *storages) {
   
    get_storages(storages);
    
    print_title("storage");
    for (int i = 0; i < 32; i++) {
        struct storage_info storage;

        storage = storages[i];

        if (storage.total <= 0 ) {
            break;
        }

        int used_percent = 100.0 * storage.used / storage.total;

        print_progress(storage.mount_point, storage.used, storage.total);
        printf(" (%d%s / %d%s) (%d%%)\n", storage.used, storage.size_suffix, storage.total, storage.size_suffix, used_percent);
    }

    printf("\n");
    
}
