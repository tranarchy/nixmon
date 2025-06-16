#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "../include/info.h"
#include "../include/cpu.h"

#define CPU_IDLE_STATE 3
#define CPU_STATES_NUM 10

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

char hwmon_path[1024] = { 0 };

int get_cpu_usage(struct cpu_info *cpu) {
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

    get_cpu_usage_percent(cpu, cpu_usage, cpu_usage_prev, CPU_STATES_NUM, CPU_IDLE_STATE);

    return 0;
}

int get_cpu_freq(struct cpu_info *cpu) {
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

    if (cpu->freq > cpu->freq_max) {
        cpu->freq_max = cpu->freq;
    }

    return 0;
}

void get_hwmon_path(void) {
    FILE *fp;

    DIR *hwmon_dir;
    DIR *hwmon_sub_dir;
        
    struct dirent *hwmon_entry;
    struct dirent *hwmon_sub_entry;

    char temp_label_buff[16];
    char hwmon_path_buff[512];
    char input_path_buff[1024];
    char label_path_buff[1024];

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

                    strlcpy(hwmon_path, input_path_buff, 1024);

                    break;
                }
            } else {
                if (strstr(hwmon_sub_entry->d_name, "label") != NULL) {
                    snprintf(label_path_buff, 1024, "%s/%s", hwmon_path_buff, hwmon_sub_entry->d_name);

                    fp = fopen(label_path_buff, "r");

                    if (fp == NULL) {
                        continue;
                    }

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
}

int get_cpu_temp(struct cpu_info *cpu) {
    FILE *fp;

    char temp_buff[16];

    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

    if (fp == NULL) {
        if (hwmon_path[0] == 0) {
            get_hwmon_path();
        }

        fp = fopen(hwmon_path, "r");
    }

    fgets(temp_buff, 16, fp);
    fclose(fp);

    cpu->temp = atoi(temp_buff) / 1000;

    if (cpu->temp > cpu->temp_max) {
        cpu->temp_max = cpu->temp;
    }

    return 0;
}
