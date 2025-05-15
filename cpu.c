#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(__OpenBSD__)
    #include <string.h>
    #include <sys/time.h>
    #include <sys/sysctl.h>
    #include <sys/sensors.h>
#endif

#include "util/util.h"

float cpu_max_freq_hz_hit, cpu_max_freq_hz = 0;
int cpu_temp_max = 0;

void get_loadavg() {
    double loadavg[3];
    char loadavg_buff[32];

    if (getloadavg(loadavg, 3) == -1) {
        return;
    }

    snprintf(loadavg_buff, 32, "%0.2f %0.2f %0.2f", loadavg[0], loadavg[1], loadavg[2]);

    pretty_print("Load avg", loadavg_buff);
}

void get_threads() {
    char threads_buff[16];

    long threads = sysconf(_SC_NPROCESSORS_CONF);
    long threads_online = sysconf(_SC_NPROCESSORS_ONLN);

    snprintf(threads_buff, 16, "%ld (%ld)", threads, threads_online);

    pretty_print("Threads", threads_buff);
}

void get_cpu_freq() {
    char freq_buff[16];
    double freq_ghz, freq = 0;

    #if defined(__linux__)
        FILE *fp;

        char freq_max_buff[16];

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

        freq = atof(freq_buff);
        freq_ghz = freq / 1000 / 1000; // KHz
        cpu_max_freq_hz = atof(freq_max_buff);

        print_progress("CPU freq", freq, cpu_max_freq_hz);
        printf(" (%.2f GHz)\n", freq_ghz);

    #elif defined(__OpenBSD__)
        struct sensor freq_sensor = get_sensor_openbsd("cpu0", SENSOR_FREQ);

        freq = freq_sensor.value;
        freq_ghz = freq / 1E15; // uHz

        snprintf(freq_buff, 16, "%.2f GHz", freq_ghz);
        pretty_print("CPU freq", freq_buff);

    #endif

    if (freq > cpu_max_freq_hz_hit) {
        cpu_max_freq_hz_hit = freq;
    }
}

void get_cpu_freq_max() {
    #if defined(__linux__)
        print_progress("Max CPU freq", cpu_max_freq_hz_hit, cpu_max_freq_hz);
        printf(" (%.2f GHz)\n", cpu_max_freq_hz_hit / 1000 / 1000);
    #elif defined(__OpenBSD__)
        char max_freq_buff[16];

        snprintf(max_freq_buff, 16, "%.2f GHz", cpu_max_freq_hz_hit / 1E15);
        pretty_print("Max CPU freq", max_freq_buff);
    #endif
}

void get_cpu_temp() {
    char temp_buff[16];

    int temp = 0;

    #if defined(__linux__)
        FILE *fp;

        fp = fopen("/sys/class/thermal/thermal_zone1/temp", "r");

        if (fp == NULL) {
            return;
        }

        fgets(temp_buff, 16, fp);
        fclose(fp);

        temp = atoi(temp_buff) / 1000;

    #elif defined(__OpenBSD__)
        struct sensor temp_sensor = get_sensor_openbsd("cpu0", SENSOR_TEMP);

        temp = microkelvin_to_celsius(temp_sensor.value);
    #endif

    if (temp > cpu_temp_max) {
        cpu_temp_max = temp;
    }

    snprintf(temp_buff, 16, "%dC", temp);

    pretty_print("CPU temp", temp_buff);
}

void get_cpu_temp_max() {
    char temp_max_buff[16];

    if (cpu_temp_max == 0) {
        return;
    }

    snprintf(temp_max_buff, 16, "%dC", cpu_temp_max);

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
