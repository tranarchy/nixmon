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
    #define CPU_IDLE_STATE 3
    #define CPU_STATES_NUM 10
#elif defined(__OpenBSD__)
    #define CPU_IDLE_STATE 5 
    #define CPU_STATES_NUM 6
#else
    #define CPU_IDLE_STATE 4
    #define CPU_STATES_NUM 5
#endif

#if defined(__FreeBSD__)
    #define SYSCTL_FREQ_NAME "dev.cpu.0.freq"
#elif defined(__NetBSD__)
    #define SYSCTL_FREQ_NAME "machdep.cpu.frequency.current"
#endif

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

double cpu_freq_hit_max, cpu_freq_max = 0;

int cpu_temp_max, cpu_usage_percent_max = 0;

int get_loadavg(void) {
    double loadavg[3];
    char loadavg_buff[32];

    int ret = getloadavg(loadavg, 3);

    if (ret == -1) {
        return ret;
    }

    snprintf(loadavg_buff, 32, "%0.2f %0.2f %0.2f", loadavg[0], loadavg[1], loadavg[2]);

    pretty_print("Load avg", loadavg_buff);

    return 0;
}

int get_threads(void) {
    char threads_buff[16];

    long threads = sysconf(_SC_NPROCESSORS_CONF);
    long threads_online = sysconf(_SC_NPROCESSORS_ONLN);

    snprintf(threads_buff, 16, "%ld (%ld)", threads, threads_online);

    pretty_print("Threads", threads_buff);

    return 0;
}

int get_cpu_usage(void) {

    int cpu_usage_percent = 0;
    
    long long cpu_idle = 0, cpu_usage_sum = 0, cpu_non_idle_usage_sum = 0;

    #if defined(__linux__)

        FILE *fp;
        char line[512];

        fp = fopen("/proc/stat", "r");

        if (fp == NULL) {
            return -1;
        }

        if (fgets(line, sizeof(line), fp) == NULL) {
            return -1;
        }

        fclose(fp);

        sscanf(
            line, 
            "cpu %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
            &cpu_usage[0], &cpu_usage[1], &cpu_usage[2], &cpu_usage[3], &cpu_usage[4], &cpu_usage[5], &cpu_usage[6], &cpu_usage[7], &cpu_usage[8], &cpu_usage[9]
        );

    #elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)

        size_t len;
        len = sizeof(cpu_usage);

        #if defined(__OpenBSD__)
            int mib[2];

            mib[0] = CTL_KERN;
            mib[1] = KERN_CPTIME;

            int ret = sysctl(mib, 2, &cpu_usage, &len, NULL, 0);
        #else
            int ret = sysctlbyname("kern.cp_time", &cpu_usage, &len, NULL, 0);
        #endif

        if (ret == -1) {
            return ret;
        }
        
    #endif

    for (int i = 0; i < CPU_STATES_NUM; i++) {
        cpu_usage_sum += cpu_usage[i] - cpu_usage_prev[i];
    }

    cpu_idle = cpu_usage[CPU_IDLE_STATE] - cpu_usage_prev[CPU_IDLE_STATE];
    cpu_non_idle_usage_sum = cpu_usage_sum - cpu_idle;

    cpu_usage_percent = 100.0 * cpu_non_idle_usage_sum / cpu_usage_sum;
        
    for (int i = 0; i < CPU_STATES_NUM; i++) {
        cpu_usage_prev[i] = cpu_usage[i];
    }

    if (cpu_usage_percent > cpu_usage_percent_max) {
        cpu_usage_percent_max = cpu_usage_percent;
    }

    print_progress("CPU usage", cpu_usage_percent, 100);
    printf(" (%d%%)\n", cpu_usage_percent);

    return 0;
}

int get_cpu_usage_max(void) {
    print_progress("Max CPU usage", cpu_usage_percent_max, 100);
    printf(" (%d%%)\n", cpu_usage_percent_max);

    return 0;
}

int get_cpu_freq(void) {
    char freq_buff[16];
    double freq_ghz, freq = 0;

    #if defined(__linux__)
        FILE *fp;

        char freq_max_buff[16];

        fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");

        if (fp == NULL) {
            return -1;
        }

        fgets(freq_buff, 16, fp);
        fclose(fp);

        fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq", "r");

        if (fp == NULL) {
            return -1;
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
        int ret = sysctlbyname(SYSCTL_FREQ_NAME, &freq_bsd, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        freq = freq_bsd;
        freq_ghz = freq / 1000;

        snprintf(freq_buff, 16, "%.2f GHz", freq_ghz);
        pretty_print("CPU freq", freq_buff);
    #endif

    if (freq > cpu_freq_hit_max) {
        cpu_freq_hit_max = freq;
    }

    return 0;
}

int get_cpu_freq_max(void) {
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

    return 0;
}

int get_cpu_temp(void) {
    char temp_buff[16];

    int temp = 0;

    #if defined(__linux__)
        FILE *fp;

        fp = fopen("/sys/class/thermal/thermal_zone1/temp", "r");

        if (fp == NULL) {
            return -1;
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
            return ret;
        }

        temp = temp / 10 - 273.15;
    #endif

    if (temp > cpu_temp_max) {
        cpu_temp_max = temp;
    }

    snprintf(temp_buff, 16, "%d°C", temp);

    pretty_print("CPU temp", temp_buff);

    return 0;
}

int get_cpu_temp_max(void) {
    char temp_max_buff[16];

    if (cpu_temp_max == 0) {
        return -1;
    }

    snprintf(temp_max_buff, 16, "%d°C", cpu_temp_max);

    pretty_print("Max CPU temp", temp_max_buff);

    return 0;
}

int cpu_init(void) {
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

    return 0;
}
