#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(__linux__)
    #include <dirent.h>
    #include <string.h>
#endif

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

#include "info.h"

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

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

int get_loadavg(struct cpu_info *cpu) {
    int ret = getloadavg(cpu->loadavg, 3);

    if (ret == -1) {
        return ret;
    }

    return 0;
}

int get_threads(struct cpu_info *cpu) {
    cpu->threads = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu->threads == -1) {
        return -1;
    }

    cpu->threads_online = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu->threads_online == -1) {
        return -1;
    }

    return 0;
}

int get_cpu_usage(struct cpu_info *cpu) {
    
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

    cpu->usage = 100.0 * cpu_non_idle_usage_sum / cpu_usage_sum;
        
    for (int i = 0; i < CPU_STATES_NUM; i++) {
        cpu_usage_prev[i] = cpu_usage[i];
    }

    if ( cpu->usage > cpu->usage_max) {
        cpu->usage_max =  cpu->usage;
    }

    return 0;
}

int get_cpu_freq(struct cpu_info *cpu) {
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

        cpu->freq = atof(freq_buff) / 1000 / 1000; // KHz
        cpu->freq_total = atof(freq_max_buff) / 1000 / 1000;

    #elif defined(__OpenBSD__)

        struct sensor freq_sensor = get_sensor_openbsd("cpu0", SENSOR_FREQ);

        cpu->freq = freq_sensor.value / 1E15; // μHz

    #elif defined(__FreeBSD__) || defined(__NetBSD__)

        size_t len;
        int freq_bsd;

        len = sizeof(freq_bsd);
        int ret = sysctlbyname(SYSCTL_FREQ_NAME, &freq_bsd, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        cpu->freq = freq_bsd / 1000;
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

        cpu->freq = ((double)tbfreq * (double)clockinfo.hz) / 1000 / 1000 / 1000;

    #endif

    if (cpu->freq > cpu->freq_max) {
        cpu->freq_max = cpu->freq;
    }

    return 0;
}

int get_cpu_temp(struct cpu_info *cpu) {
    #if defined(__linux__)
       
        FILE *fp;

        DIR *hwmon_dir;
        DIR *hwmon_sub_dir;
        
        struct dirent *hwmon_entry;
        struct dirent *hwmon_sub_entry;

        char temp_buff[16];
        char temp_label_buff[16];
        char hwmon_path_buff[512];
        char input_path_buff[1024];
        char label_path_buff[1024];

        fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

        if (fp == NULL) {

            int right_dir = 0;
            hwmon_dir = opendir("/sys/class/hwmon");

            while ((hwmon_entry = readdir(hwmon_dir)) != NULL) {
                if (strcmp(hwmon_entry->d_name, ".") == 0) {
                    continue;
                } else if (strcmp(hwmon_entry->d_name, "..") == 0) {
                    continue;
                }

                snprintf(hwmon_path_buff, 512, "/sys/class/hwmon/%s", hwmon_entry->d_name);

                hwmon_sub_dir = opendir(hwmon_path_buff);

                while ((hwmon_sub_entry = readdir(hwmon_sub_dir)) != NULL) {
                    if (strcmp(hwmon_sub_entry->d_name, ".") == 0) {
                        continue;
                    } else if (strcmp(hwmon_sub_entry->d_name, "..") == 0) {
                        continue;
                    }

                    if (right_dir) {
                        if (strstr(hwmon_sub_entry->d_name, "input") != NULL) {
                            snprintf(input_path_buff, 1024, "%s/%s", hwmon_path_buff, hwmon_sub_entry->d_name);
                            
                            char input_number = input_path_buff[strlen(input_path_buff) - 7];
                            char label_number = label_path_buff[strlen(label_path_buff) - 7];

                            if (input_number != label_number) {
                                continue;
                            }

                            fp = fopen(input_path_buff, "r");

                            break;
                        }
                    } else {
                        if (strstr(hwmon_sub_entry->d_name, "label") != NULL) {
                            snprintf(label_path_buff, 1024, "%s/%s", hwmon_path_buff, hwmon_sub_entry->d_name);

                            fp = fopen(label_path_buff, "r");

                            fgets(temp_label_buff, 16, fp);

                            if (strstr(temp_label_buff, "Tctl") != NULL || strstr(temp_label_buff, "Tdie") != NULL) {
                                right_dir = 1;
                                rewinddir(hwmon_sub_dir);
                            }
                        }
                    }
                }

                closedir(hwmon_sub_dir);

                if (right_dir) {
                    break;
                }
            }

            closedir(hwmon_dir);

            if (!right_dir) {
                return -1;
            }
        }

        fgets(temp_buff, 16, fp);
        fclose(fp);

        cpu->temp = atoi(temp_buff) / 1000;

    #elif defined(__OpenBSD__)

        struct sensor temp_sensor = get_sensor_openbsd("cpu0", SENSOR_TEMP);

        cpu->temp = microkelvin_to_celsius(temp_sensor.value);

    #elif defined(__FreeBSD__)

        size_t len;

        len = sizeof(cpu->temp);
        int ret = sysctlbyname("dev.cpu.0.temperature", &cpu->temp, &len, NULL, 0);

        if (ret == -1) {
            return ret;
        }

        cpu->temp = temp / 10 - 273.15;

    #elif defined(__APPLE__)
        cpu->temp = get_macos_cpu_temp(0xff00, 5);
    #endif

    if (cpu->temp > cpu->temp_max) {
        cpu->temp_max = cpu->temp;
    }

    return 0;
}

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
        #if defined(__linux__)
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
