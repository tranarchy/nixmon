#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

#include "util/util.h"

#define CARD0_PATH_I915 "/sys/class/drm/card0/"

int i915_freq_max, i915_freq_cap = 0;

void get_i915_gpu_freq() {
    FILE *fp;

    char freq_buff[32], freq_cap_buff[32], freq_path_buff[512], freq_cap_path_buff[512];

    int freq = 0;

    sprintf(freq_path_buff, "%sgt_act_freq_mhz", CARD0_PATH_I915);

    fp = fopen(freq_path_buff, "r");

    if (fp == NULL) {
        return;
    }

    fgets(freq_buff, 32, fp);
    fclose(fp);

    sprintf(freq_cap_path_buff, "%sgt_max_freq_mhz", CARD0_PATH_I915);

    fp = fopen(freq_cap_path_buff, "r");

    if (fp == NULL) {
        return;
    }

    fgets(freq_cap_buff, 32, fp);
    fclose(fp);

    freq = atoi(freq_buff);
    i915_freq_cap = atoi(freq_cap_buff);

    if (freq > i915_freq_max) {
        i915_freq_max = freq;
    }

    print_progress("GPU freq", freq, i915_freq_cap);
    printf(" (%dMHz / %dMHz)\n", freq, i915_freq_cap);
}

void get_i915_gpu_freq_max() {
    print_progress("Max GPU freq", i915_freq_max, i915_freq_cap);
    printf(" (%dMHz / %dMHz)\n", i915_freq_max, i915_freq_cap);
}

int i915_exists() {
    DIR *dp;

    dp = opendir("/sys/devices/i915");

    if (dp == NULL) {
        return 0;
    }

    closedir(dp);

    return 1;
}

void i915_init() {

    if (!i915_exists()) {
        return;
    }

    pretty_print_title("gpu (i915)");
    get_i915_gpu_freq();
    get_i915_gpu_freq_max();
    printf("\n");
}