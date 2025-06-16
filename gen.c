#include <stdio.h>
#include <sys/utsname.h>

#include "include/info.h"
#include "include/util.h"
#include "include/gen.h"

void gen_init(struct gen_info *gen) {
    struct utsname utsname_buff;

    print_title("gen");
    if (uname(&utsname_buff) != -1) {
        pretty_print("OS", utsname_buff.sysname);
        pretty_print("Kernel", utsname_buff.release);
        pretty_print("Arch", utsname_buff.machine);
        printf("\n");
    }

    if (get_uptime(gen) != -1) {
        char uptime_buff[32];
        snprintf(uptime_buff, 32, "%dd %dh %dm %ds", 
            gen->uptime_day, 
            gen->uptime_hour, 
            gen->uptime_min,
            gen->uptime_sec
        );
        pretty_print("Uptime", uptime_buff);
        printf("\n");
    }
}
