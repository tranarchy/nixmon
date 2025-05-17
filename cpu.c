#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #include <sys/sysctl.h>
#endif

#if defined(__OpenBSD__)
    #include <string.h>
    #include <sys/time.h>
    #include <sys/sensors.h>
#endif

#include "util/util.h"

#if defined(__linux__)
    long long cpu_usage[10];
    long long cpu_usage_prev[10] = { 0 };
#else
    long long cpu_usage[5];
    long long cpu_usage_prev[5] = { 0 };
#endif

double cpu_freq_hit_max, cpu_freq_max = 0;

int cpu_temp_max, cpu_usage_percent_max = 0;

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

void get_cpu_usage() {

    int cpu_usage_percent = 0;
    
    long long cpu_idle = 0, cpu_usage_sum = 0, cpu_non_idle_usage_sum = 0;

    #if defined(__linux__)

        FILE *fp;
        char line[512];

        fp = fopen("/proc/stat", "r");

        if (fp == NULL) {
            return;
        }

        if (fgets(line, sizeof(line), fp) == NULL) {
            return;
        }

        fclose(fp);

        sscanf(
            line, 
            "cpu %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
            &cpu_usage[0], &cpu_usage[1], &cpu_usage[2], &cpu_usage[3], &cpu_usage[4], &cpu_usage[5], &cpu_usage[6], &cpu_usage[7], &cpu_usage[8], &cpu_usage[9]
        );

        for (int i = 0; i < 10; i++) {
            cpu_usage_sum += cpu_usage[i] - cpu_usage_prev[i];
        }

        cpu_idle = cpu_usage[3] - cpu_usage_prev[3];
        cpu_non_idle_usage_sum = cpu_usage_sum - cpu_idle;

        cpu_usage_percent = 100.0 * cpu_non_idle_usage_sum / cpu_usage_sum;
        
        for (int i = 0; i < 10; i++) {
            cpu_usage_prev[i] = cpu_usage[i];
        }

    #elif defined(__FreeBSD__) || defined(__NetBSD__)

        size_t len;
        len = sizeof(cpu_usage);

        int ret = sysctlbyname("kern.cp_time", &cpu_usage, &len, NULL, 0);

        if (ret == -1) {
            return;
        }

        for (int i = 0; i < 5; i++) {
            cpu_usage_sum += cpu_usage[i] - cpu_usage_prev[i];
        }

        cpu_idle = cpu_usage[4] - cpu_usage_prev[4];
        cpu_non_idle_usage_sum = cpu_usage_sum - cpu_idle;

        cpu_usage_percent = 100.0 * cpu_non_idle_usage_sum / cpu_usage_sum;
        
        for (int i = 0; i < 5; i++) {
            cpu_usage_prev[i] = cpu_usage[i];
        }

        
    #endif

    if (cpu_usage_percent > cpu_usage_percent_max) {
        cpu_usage_percent_max = cpu_usage_percent;
    }

    print_progress("CPU usage", cpu_usage_percent, 100);
    printf(" (%d%%)\n", cpu_usage_percent);
}

void get_cpu_usage_max() {
    print_progress("Max CPU usage", cpu_usage_percent_max, 100);
    printf(" (%d%%)\n", cpu_usage_percent_max);
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
        cpu_freq_max = atof(freq_max_buff);

        print_progress("CPU freq", freq, cpu_freq_max);
        printf(" (%.2f GHz)\n", freq_ghz);

    #elif defined(__OpenBSD__)
        struct sensor freq_sensor = get_sensor_openbsd("cpu0", SENSOR_FREQ);

        freq = freq_sensor.value;
        freq_ghz = freq / 1E15; // μHz

        snprintf(freq_buff, 16, "%.2f GHz", freq_ghz);
        pretty_print("CPU freq", freq_buff);
    #elif defined(__FreeBSD__) || defined(__NetBSD__)
        size_t len;
        int freq_bsd;

        len = sizeof(freq_bsd);
        #if defined(__FreeBSD__)
            int ret = sysctlbyname("dev.cpu.0.freq", &freq_bsd, &len, NULL, 0);
        #else
            int ret = sysctlbyname("machdep.cpu.frequency.current", &freq_bsd, &len, NULL, 0);
        #endif

        if (ret == -1) {
            return;
        }

        freq = freq_bsd;
        freq_ghz = freq / 1000;

        snprintf(freq_buff, 16, "%.2f GHz", freq_ghz);
        pretty_print("CPU freq", freq_buff);
    #endif

    if (freq > cpu_freq_hit_max) {
        cpu_freq_hit_max = freq;
    }
}

void get_cpu_freq_max() {
    #if defined(__linux__)
        print_progress("Max CPU freq", cpu_freq_hit_max, cpu_freq_max);
        printf(" (%.2f GHz)\n", cpu_freq_hit_max / 1000 / 1000);
    #elif defined(__OpenBSD__)
        char max_freq_buff[16];

        snprintf(max_freq_buff, 16, "%.2f GHz", cpu_freq_hit_max / 1E15);
        pretty_print("Max CPU freq", max_freq_buff);
    #elif defined(__FreeBSD__) || defined(__NetBSD__)
        char max_freq_buff[16];

        snprintf(max_freq_buff, 16, "%.2f GHz", cpu_freq_hit_max / 1000);
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
    #elif defined(__FreeBSD__)
        size_t len;

        len = sizeof(temp);
        int ret = sysctlbyname("dev.cpu.0.temperature", &temp, &len, NULL, 0);

        if (ret == -1) {
            return;
        }

        temp = temp / 10 - 273.15;
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
    get_cpu_usage();
    get_cpu_usage_max();
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
