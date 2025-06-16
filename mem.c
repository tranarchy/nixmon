#include <stdio.h>

#include "include/util.h"
#include "include/info.h"
#include "include/mem.h"

void mem_init(struct mem_info *mem) {

    if (get_mem_usage(mem) != -1) {
        print_title("mem");
        print_progress("RAM usage", mem->used, mem->total);
        printf(" (%dMiB / %dMiB)\n",  mem->used, mem->total);
    
        print_progress("Max RAM usage", mem->used_max, mem->total);
        printf(" (%dMiB / %dMiB)\n",  mem->used_max, mem->total);
        printf("\n");
    }
}
