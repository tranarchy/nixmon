#include <stdio.h>

#include "include/util.h"
#include "include/info.h"
#include "include/cpu.h"

void cpu_init(struct cpu_info *cpu) {

    print_title("cpu");
    if (get_threads(cpu) != -1) {
        char threads_buff[16];
        snprintf(threads_buff, 16, "%ld (%ld)", cpu->threads, cpu->threads_online);
        pretty_print("Threads", threads_buff);
        printf("\n");
    }
   
    if (get_cpu_usage(cpu) != -1) {
        print_progress("CPU usage", cpu->usage, 100);
        printf(" (%d%%)\n", cpu->usage);

        print_progress("Max CPU usage", cpu->usage_max, 100);
        printf(" (%d%%)\n", cpu->usage_max);
        printf("\n");
    }
   
    if (get_cpu_freq(cpu) != -1) {
        #ifdef __linux__
            print_progress("CPU freq", cpu->freq , cpu->freq_total);
            printf(" (%.2f GHz)\n", cpu->freq);

            print_progress("Max CPU freq", cpu->freq_max, cpu->freq_total);
            printf(" (%.2f GHz)\n", cpu->freq_max);
        #else
            char freq_buff[16];

            snprintf(freq_buff, 16, "%.2f GHz", cpu->freq);
            pretty_print("CPU freq", freq_buff);

            snprintf(freq_buff, 16, "%.2f GHz", cpu->freq_max);
            pretty_print("Max CPU freq", freq_buff);
        #endif

        printf("\n");
    }

    if (get_cpu_temp(cpu) != -1) {
        char temp_buff[16];
        snprintf(temp_buff, 16, "%d°C", cpu->temp);
        pretty_print("CPU temp", temp_buff);

        snprintf(temp_buff, 16, "%d°C", cpu->temp_max);
        pretty_print("Max CPU temp", temp_buff);
        printf("\n");
    }
   
    if (get_loadavg(cpu) != -1) {
        char loadavg_buff[32];

        snprintf(loadavg_buff, 32, "%0.2f %0.2f %0.2f", cpu->loadavg[0], cpu->loadavg[1], cpu->loadavg[2]);
        pretty_print("Load avg", loadavg_buff);
        printf("\n");
    }
    
}
