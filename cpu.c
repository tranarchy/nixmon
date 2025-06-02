#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
    #include <sys/sysctl.h>
#endif

#if defined(__APPLE__)
    #include <mach/mach.h>
    
    #include "apple/cpu_temp.h"
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
#elif defined(__APPLE__)
    #define CPU_IDLE_STATE 2
    #define CPU_STATES_NUM 4
#else
    #define CPU_IDLE_STATE 4
    #define CPU_STATES_NUM 5
#endif

#if defined(__FreeBSD__)
    #define SYSCTL_FREQ_NAME "dev.cpu.0.freq"
#elif defined(__NetBSD__)
    #define SYSCTL_FREQ_NAME "machdep.cpu.frequency.current"
#endif

struct cpu_info {
    long threads;
    long threads_online;

    int usage;
    int usage_max;

    double freq;
    double freq_max;

    #if defined(__linux__)
        double freq_total;
    #endif

    int temp;
    int temp_max;

    double loadavg[3];
};

struct cpu_info cpu_info;

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

int get_loadavg(struct cpu_info *cpu_info) {
    int ret = getloadavg(cpu_info->loadavg, 3);

    if (ret == -1) {
        return ret;
    }

    return 0;
}

int get_threads(struct cpu_info *cpu_info) {
    cpu_info->threads = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_info->threads == -1) {
        return -1;
    }

    cpu_info->threads_online = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_info->threads_online == -1) {
        return -1;
    }

    return 0;
}

int get_cpu_usage(struct cpu_info *cpu_info) {
    
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
    #elif defined(__APPLE__)

        natural_t proc_count;
        mach_msg_type_number_t len = HOST_VM_INFO64_COUNT;
        processor_cpu_load_info_t cpu_load_info;

        int ret = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &proc_count, (processor_info_array_t *)&cpu_load_info, &len);

        if (ret == -1) {
            return ret;
        }

        for (int i = 0; i < CPU_STATES_NUM; i++) {
            cpu_usage[i] = 0;
            for (unsigned int j = 0; j < proc_count; j++) {
                cpu_usage[i] += cpu_load_info[j].cpu_ticks[i];
            }
        }

    #endif

    for (int i = 0; i < CPU_STATES_NUM; i++) {
        cpu_usage_sum += cpu_usage[i] - cpu_usage_prev[i];
    }

    cpu_idle = cpu_usage[CPU_IDLE_STATE] - cpu_usage_prev[CPU_IDLE_STATE];
    cpu_non_idle_usage_sum = cpu_usage_sum - cpu_idle;

    cpu_info->usage = 100.0 * cpu_non_idle_usage_sum / cpu_usage_sum;
        
    for (int i = 0; i < CPU_STATES_NUM; i++) {
        cpu_usage_prev[i] = cpu_usage[i];
    }

    if ( cpu_info->usage > cpu_info->usage_max) {
        cpu_info->usage_max =  cpu_info->usage;
    }

    return 0;
}

int get_cpu_freq(struct cpu_info *cpu_info) {
    #if defined(__linux__)

        FILE *fp;

        char freq_buff[16];
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

        cpu_info->freq = atof(freq_buff) / 1000 / 1000; // KHz
        cpu_info->freq_total = atof(freq_max_buff) / 1000 / 1000;

    #elif defined(__OpenBSD__)

        struct sensor freq_sensor = get_sensor_openbsd("cpu0", SENSOR_FREQ);

        cpu_info->freq = freq_sensor.value / 1E15; // μHz

    #elif defined(__FreeBSD__) || defined(__NetBSD__)

        size_t len;
        int freq_bsd;

        len = sizeof(freq_bsd);
        int ret = sysctlbyname(SYSCTL_FREQ_NAME, &freq_bsd, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        cpu_info->freq = freq_bsd / 1000;
    #elif defined(__APPLE__)
        // this is probably not the right way but apple deprecated hw.cpufrequency...

        int mib[2];

        int tbfreq;
        size_t len, slen;
        struct clockinfo clockinfo;

        mib[0] = CTL_HW;
        mib[1] = HW_TB_FREQ;

        len = sizeof(tbfreq);

        int ret = sysctl(mib, 2, &tbfreq, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        mib[0] = CTL_KERN;
        mib[1] = KERN_CLOCKRATE;

        slen = sizeof(struct clockinfo);

        ret = sysctl(mib, 2, &clockinfo, &slen, NULL, 0);

         if (ret == -1) {
            return ret;
        }

        cpu_info->freq = ((double)tbfreq * (double)clockinfo.hz) / 1000 / 1000 / 1000;

    #endif

    if (cpu_info->freq > cpu_info->freq_max) {
        cpu_info->freq_max = cpu_info->freq;
    }

    return 0;
}

int get_cpu_temp(struct cpu_info *cpu_info) {
    #if defined(__linux__)
       
        FILE *fp;

        char temp_buff[16];

        fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

        if (fp == NULL) {
            return -1;
        }

        fgets(temp_buff, 16, fp);
        fclose(fp);

        cpu_info->temp = atoi(temp_buff) / 1000;

    #elif defined(__OpenBSD__)

        struct sensor temp_sensor = get_sensor_openbsd("cpu0", SENSOR_TEMP);

        cpu_info->temp = microkelvin_to_celsius(temp_sensor.value);

    #elif defined(__FreeBSD__)

        size_t len;

        len = sizeof(cpu_info->temp);
        int ret = sysctlbyname("dev.cpu.0.temperature", &cpu_info->temp, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        cpu_info->temp = temp / 10 - 273.15;

    #elif defined(__APPLE__)
        cpu_info->temp = get_macos_cpu_temp(0xff00, 5);
    #endif

    if (cpu_info->temp > cpu_info->temp_max) {
        cpu_info->temp_max = cpu_info->temp;
    }

    return 0;
}

void cpu_init(void) {

    print_title("cpu");
    if (get_threads(&cpu_info) != -1) {
        char threads_buff[16];
        snprintf(threads_buff, 16, "%ld (%ld)", cpu_info.threads, cpu_info.threads_online);
        pretty_print("Threads", threads_buff);
        printf("\n");
    }
   
    if (get_cpu_usage(&cpu_info) != -1) {
        print_progress("CPU usage", cpu_info.usage, 100);
        printf(" (%d%%)\n", cpu_info.usage);

        print_progress("Max CPU usage", cpu_info.usage_max, 100);
        printf(" (%d%%)\n", cpu_info.usage_max);
        printf("\n");
    }
   
    if (get_cpu_freq(&cpu_info) != -1) {
        #if defined(__linux__)
            print_progress("CPU freq", cpu_info.freq , cpu_info.freq_total);
            printf(" (%.2f GHz)\n", cpu_info.freq);

            print_progress("Max CPU freq", cpu_info.freq_max, cpu_info.freq_total);
            printf(" (%.2f GHz)\n", cpu_info.freq_max);
        #else
            char freq_buff[16];

            snprintf(freq_buff, 16, "%.2f GHz", cpu_info.freq);
            pretty_print("CPU freq", freq_buff);

            snprintf(freq_buff, 16, "%.2f GHz", cpu_info.freq_max);
            pretty_print("Max CPU freq", freq_buff);
        #endif

        printf("\n");
    }

    if (get_cpu_temp(&cpu_info) != -1) {
        char temp_buff[16];
        snprintf(temp_buff, 16, "%d°C", cpu_info.temp);
        pretty_print("CPU temp", temp_buff);

        snprintf(temp_buff, 16, "%d°C", cpu_info.temp_max);
        pretty_print("Max CPU temp", temp_buff);
        printf("\n");
    }
   
    if (get_loadavg(&cpu_info) != -1) {
        char loadavg_buff[32];

        snprintf(loadavg_buff, 32, "%0.2f %0.2f %0.2f", cpu_info.loadavg[0], cpu_info.loadavg[1], cpu_info.loadavg[2]);
        pretty_print("Load avg", loadavg_buff);
        printf("\n");
    }
    
}
