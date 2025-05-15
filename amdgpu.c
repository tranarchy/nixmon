#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "util/util.h"

#define CARD0_PATH_AMDGPU "/sys/class/drm/card0/device/"

long int amd_mem_used_max, amd_mem_total, amd_freq_max = 0;
int amd_temp_max, amd_power_max, amd_power_cap = 0;

void get_amd_gpu_mem_usage() {
    FILE *fp;
    char amd_mem_total_buff[32], mem_used_buff[32];
    long int mem_used = 0;

    fp = fopen(CARD0_PATH_AMDGPU "mem_info_vram_total", "r");

    if (fp == NULL) {
        return;
    }

    fgets(amd_mem_total_buff, 32, fp);
    fclose(fp);

    fp = fopen(CARD0_PATH_AMDGPU "mem_info_vram_used", "r");

    if (fp == NULL) {
        return;
    }

    fgets(mem_used_buff, 32, fp);
    fclose(fp);


    amd_mem_total = atol(amd_mem_total_buff);
    mem_used = atol(mem_used_buff);

    if (mem_used > amd_mem_used_max) {
        amd_mem_used_max = mem_used;
    }

    print_progress("VRAM usage", mem_used, amd_mem_total);
    printf(" (%dMiB / %dMiB)\n", get_mib(mem_used), get_mib(amd_mem_total));
}

void get_amd_gpu_mem_usage_max() {
    print_progress("Max VRAM usage", amd_mem_used_max, amd_mem_total);
    printf(" (%dMiB / %dMiB)\n", get_mib(amd_mem_used_max), get_mib(amd_mem_total));
}

char *get_amd_hwmon() {
    DIR *dp;
    struct dirent *ep;

    dp = opendir(CARD0_PATH_AMDGPU "hwmon");

    if (dp == NULL) {
        return NULL;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (strstr(ep->d_name, "hwmon") != NULL) {
            break;
        }
    }

    closedir(dp);

    return ep->d_name;
}

void get_amd_gpu_power() {
    FILE *fp;

    char power_buff[32],  amd_power_cap_buff[32], power_path_buff[512], amd_power_cap_path_buff[512];

    int power = 0;

    char *amd_hwmon =  get_amd_hwmon();

    if (amd_hwmon == NULL) {
        return;
    }

    snprintf(power_path_buff, 512, "%shwmon/%s/power1_average", CARD0_PATH_AMDGPU, amd_hwmon);

    fp = fopen(power_path_buff, "r");

    if (fp == NULL) {
        return;
    }

    fgets(power_buff, 32, fp);
    fclose(fp);

    snprintf(amd_power_cap_path_buff, 512, "%shwmon/%s/power1_cap_max", CARD0_PATH_AMDGPU, amd_hwmon);

    fp = fopen(amd_power_cap_path_buff, "r");

    if (fp == NULL) {
        return;
    }

    fgets(amd_power_cap_buff, 32, fp);
    fclose(fp);

    power = atoi(power_buff) / 1000 / 1000;
    amd_power_cap = atoi(amd_power_cap_buff) / 1000 / 1000;

    if (power > amd_power_max) {
        amd_power_max = power;
    }

    print_progress("GPU power", power, amd_power_cap);
    printf(" (%dW / %dW)\n", power, amd_power_cap);
}

void get_amd_gpu_amd_power_max() {
    print_progress("Max GPU power", amd_power_max, amd_power_cap);
    printf(" (%dW / %dW)\n", amd_power_max, amd_power_cap);
}

void get_amd_gpu_temp() {
    FILE *fp;

    char temp_buff[32], temp_path_buff[512];

    int temp = 0;

    char *amd_hwmon =  get_amd_hwmon();

    if (amd_hwmon == NULL) {
        return;
    }

    snprintf(temp_path_buff, 512, "%shwmon/%s/temp1_input", CARD0_PATH_AMDGPU, amd_hwmon);

    fp = fopen(temp_path_buff, "r");

    if (fp == NULL) {
        return;
    }

    fgets(temp_buff, 32, fp);
    fclose(fp);

    temp = atoi(temp_buff) / 1000;

    if (temp > amd_temp_max) {
        amd_temp_max = temp;
    }

    snprintf(temp_buff, 32, "%dC", temp);

    pretty_print("GPU temp", temp_buff);
}

void get_amd_gpu_amd_temp_max() {
    char temp_buff[32];
    snprintf(temp_buff, 32, "%dC", amd_temp_max);
    pretty_print("Max GPU temp", temp_buff);
}

void get_amd_gpu_freq() {
    FILE *fp;

    char freq_buff[32], freq_path_buff[512];

    long int freq = 0;

    char *amd_hwmon =  get_amd_hwmon();

    if (amd_hwmon == NULL) {
        return;
    }

    snprintf(freq_path_buff, 512, "%shwmon/%s/freq1_input", CARD0_PATH_AMDGPU, amd_hwmon);

    fp = fopen(freq_path_buff, "r");

    if (fp == NULL) {
        return;
    }

    fgets(freq_buff, 32, fp);
    fclose(fp);

    freq = atol(freq_buff) / 1000 / 1000;

    if (freq > amd_freq_max) {
        amd_freq_max = freq;
    }

    snprintf(freq_buff, 32, "%ld MHz", freq);

    pretty_print("GPU freq", freq_buff);
}

void get_amd_gpu_amd_freq_max() {
    char amd_freq_max_buff[32];
    snprintf(amd_freq_max_buff, 32, "%ld MHz", amd_freq_max);
    pretty_print("Max GPU freq", amd_freq_max_buff);
}

int amd_exists() {
    FILE *fp;

    char name_buff[32], name_path_buff[512];

    char *amd_hwmon =  get_amd_hwmon();

    if (amd_hwmon == NULL) {
        return 0;
    }

    snprintf(name_path_buff, 512, "%shwmon/%s/name", CARD0_PATH_AMDGPU, amd_hwmon);

    fp = fopen(name_path_buff, "r");

    if (fp == NULL) {
        return 0;
    }

    fgets(name_buff, 32, fp);
    fclose(fp);

    if (strcmp(name_buff, "amdgpu") >= 0) {
        return 1;
    }

    return 0;
}

void amd_gpu_init() {

    if (!amd_exists()) {
        return;
    }

    pretty_print_title("gpu (amdgpu)");
    get_amd_gpu_mem_usage();
    get_amd_gpu_mem_usage_max();
    printf("\n");
    get_amd_gpu_power();
    get_amd_gpu_amd_power_max();
    printf("\n");
    get_amd_gpu_freq();
    get_amd_gpu_amd_freq_max();
    printf("\n");
    get_amd_gpu_temp();
    get_amd_gpu_amd_temp_max();
    printf("\n");
}
