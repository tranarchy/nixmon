#include <stdio.h>

#include <libdrm/amdgpu.h>
#include <libdrm/amdgpu_drm.h>

#include "../util/util.h"

int amdgpu_temp_max, amdgpu_freq_max, amdgpu_mem_max, amdgpu_mem_used_max, amdgpu_power_max, amdgpu_usage_max = 0;

int get_amdgpu_usage(amdgpu_device_handle device) {
    int usage;
    size_t len;

    len = sizeof(usage);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_LOAD, len, &usage);

    if (ret == -1) {
        return ret;
    }

    if (usage > amdgpu_usage_max) {
        amdgpu_usage_max = usage;
    }

    print_progress("GPU usage", usage, 100);
    printf(" (%d%%)\n", usage);

    return 0;
}

int get_amdgpu_usage_max(void) {
    print_progress("Max GPU usage", amdgpu_usage_max, 100);
    printf(" (%d%%)\n", amdgpu_usage_max);

    return 0;
}

int get_amdgpu_mem_usage(amdgpu_device_handle device) {
    long long mem, mem_max;
    size_t len, slen;

    len = sizeof(mem);
    int ret = amdgpu_query_info(device, AMDGPU_INFO_VRAM_USAGE, len, &mem);

    if (ret == -1) {
        return ret;
    }

    slen = sizeof(mem_max);
    ret = amdgpu_query_info(device, AMDGPU_INFO_MEMORY, slen, &mem_max);

    if (ret == -1) {
        return ret;
    }

    amdgpu_mem_max = get_mib(mem_max);
    mem = get_mib(mem);

    if (mem > amdgpu_mem_used_max) {
        amdgpu_mem_used_max = mem;
    }
   
    print_progress("VRAM usage", mem, amdgpu_mem_max);
    printf(" (%lldMiB / %dMiB)\n", mem, amdgpu_mem_max);

    return 0;
}

int get_amdgpu_mem_usage_max(void) {
    print_progress("Max VRAM usage", amdgpu_mem_used_max, amdgpu_mem_max);
    printf(" (%dMiB / %dMiB)\n", amdgpu_mem_used_max, amdgpu_mem_max);

    return 0;
}

int get_amdgpu_power(amdgpu_device_handle device) {
    int power;
    size_t len;

    char power_buff[32];

    len = sizeof(power);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_AVG_POWER, len, &power);

    if (ret == -1) {
        return ret;
    }

    if (power > amdgpu_power_max) {
        amdgpu_power_max = power;
    }

    snprintf(power_buff, 32, "%d W", power);
    pretty_print("GPU power", power_buff);

    return 0;
}

int get_amdgpu_power_max(void) {
    char power_buff[32];
    snprintf(power_buff, 32, "%d W", amdgpu_power_max);
    pretty_print("Max GPU power", power_buff);

    return 0;
}

int get_amdgpu_temp(amdgpu_device_handle device) {
    int temp;
    size_t len;

    char temp_buff[32];

    len = sizeof(temp);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_TEMP, len, &temp);

    if (ret == -1) {
        return ret;
    }

    temp /= 1000;

    if (temp > amdgpu_temp_max) {
        amdgpu_temp_max = temp;
    }

    snprintf(temp_buff, 32, "%d°C", temp);
    pretty_print("GPU temp", temp_buff);

    return 0;
}

int get_amdgpu_temp_max(void) {
    char temp_buff[32];
    snprintf(temp_buff, 32, "%d°C", amdgpu_temp_max);
    pretty_print("Max GPU temp", temp_buff);

    return 0;
}

int get_amdgpu_freq(amdgpu_device_handle device) {
    int freq;
    size_t len;

    char freq_buff[32];

    len = sizeof(freq);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GFX_SCLK, len, &freq);

    if (ret == -1) {
        return ret;
    }

    if (freq > amdgpu_freq_max) {
        amdgpu_freq_max = freq;
    }

    snprintf(freq_buff, 32, "%d MHz", freq);
    pretty_print("GPU freq", freq_buff);

    return 0;
}

int get_amdgpu_freq_max(void) {
    char freq_buff[32];
    snprintf(freq_buff, 32, "%d MHz", amdgpu_freq_max);
    pretty_print("Max GPU freq", freq_buff);

    return 0;
}

int get_amdgpu_name(amdgpu_device_handle device) {
    pretty_print("GPU name", amdgpu_get_marketing_name(device));

    return 0;
}

int amdgpu_init(int fd) {
    uint32_t major_version;
    uint32_t minor_version;

    amdgpu_device_handle device;

    int ret = amdgpu_device_initialize(fd, &major_version, &minor_version, &device);

    if (ret == -1) {
        return ret;
    }

    pretty_print_title("gpu (amdgpu)");
    get_amdgpu_name(device);
    printf("\n");
    get_amdgpu_usage(device);
    get_amdgpu_usage_max();
    printf("\n");
    get_amdgpu_mem_usage(device);
    get_amdgpu_mem_usage_max();
    printf("\n");
    get_amdgpu_power(device);
    get_amdgpu_power_max();
    printf("\n");
    get_amdgpu_freq(device);
    get_amdgpu_freq_max();
    printf("\n");
    get_amdgpu_temp(device);
    get_amdgpu_temp_max();
    
    amdgpu_device_deinitialize(device);

    return 0;
}
