#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "util/util.h"

float cpu_cpu_max_freq_hz_hit, cpu_max_freq_hz = 0;
int cpu_temp_max = 0;

void get_loadavg() {
    double loadavg[3];
    char loadavg_buff[32];

    if (getloadavg(loadavg, 3) == -1) {
        return;
    }

    sprintf(loadavg_buff, "%0.2f %0.2f %0.2f", loadavg[0], loadavg[1], loadavg[2]);

    pretty_print("Load avg", loadavg_buff);
}

void get_threads() {
    char threads_buff[16];

    long threads = sysconf(_SC_NPROCESSORS_CONF);
    long threads_online = sysconf(_SC_NPROCESSORS_ONLN);

    sprintf(threads_buff, "%ld (%ld)", threads, threads_online);

    pretty_print("Threads", threads_buff);
}

void get_cpu_freq() {
    #if __linux__
        FILE *fp;
        char freq_buff[16], freq_max_buff[16];
        float freq_ghz, freq_hz = 0;

        fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");

        if (fp == NULL) {
            return;
        }

        fgets(freq_buff, 16, fp);
        fclose(fp);

        fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq", "r");

        if (fp == NULL) {
            return;
        }

        fgets(freq_max_buff, 16, fp);
        fclose(fp);

        freq_hz = atof(freq_buff);
        freq_ghz = freq_hz / 1000 / 1000;
        cpu_max_freq_hz = atof(freq_max_buff);

        if (freq_hz > cpu_cpu_max_freq_hz_hit) {
            cpu_cpu_max_freq_hz_hit = freq_hz;
        }

        print_progress("CPU freq", freq_hz, cpu_max_freq_hz);

        printf(" (%.2f GHz)\n", freq_ghz);
    #endif
}

void get_cpu_freq_max() {
    print_progress("Max CPU freq", cpu_cpu_max_freq_hz_hit, cpu_max_freq_hz);
    printf(" (%.2f GHz)\n", cpu_cpu_max_freq_hz_hit / 1000 / 1000);
}

void get_cpu_temp() {
    FILE *fp;
    char temp_buff[16];

    int temp = 0;

    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

    if (fp == NULL) {
        return;
    }

    fgets(temp_buff, 16, fp);
    fclose(fp);

    temp = atoi(temp_buff) / 1000;

    if (temp > cpu_temp_max) {
        cpu_temp_max = temp;
    }

    sprintf(temp_buff, "%dC", temp);

    pretty_print("CPU temp", temp_buff);
}

void get_cpu_temp_max() {
    char temp_max_buff[16];

    if (cpu_temp_max == 0) {
        return;
    }

    sprintf(temp_max_buff, "%dC", cpu_temp_max);

    pretty_print("Max CPU temp", temp_max_buff);
}

void cpu_init() {
    pretty_print_title("cpu");
    get_threads();
    printf("\n");
    get_cpu_freq();
    get_cpu_freq_max();
    printf("\n");
    get_cpu_temp();
    get_cpu_temp_max();
    printf("\n");
    get_loadavg();
    printf("\n");
}