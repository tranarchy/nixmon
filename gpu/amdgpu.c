#include <stdio.h>
#include <string.h>

#include <libdrm/amdgpu.h>
#include <libdrm/amdgpu_drm.h>

#include "../util/util.h"

struct amdgpu_info {
    char name[64];

    int usage;
    int usage_max;

    int vram_total;

    int vram;
    int vram_max;

    int power;
    int power_max;

    int freq;
    int freq_max;

    int temp;
    int temp_max;
};

struct amdgpu_info amdgpu_info;

int get_amdgpu_usage(amdgpu_device_handle device, struct amdgpu_info *amdgpu_info) {
    size_t len;

    len = sizeof(amdgpu_info->usage);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_LOAD, len, &amdgpu_info->usage);

    if (ret < 0) {
        return ret;
    }

    if (amdgpu_info->usage > amdgpu_info->usage_max) {
        amdgpu_info->usage_max = amdgpu_info->usage;
    }

    return 0;
}

int get_amdgpu_mem_usage(amdgpu_device_handle device, struct amdgpu_info *amdgpu_info) {
    long long mem, mem_max;
    size_t len, slen;

    len = sizeof(mem);
    int ret = amdgpu_query_info(device, AMDGPU_INFO_VRAM_USAGE, len, &mem);

    if (ret < 0) {
        return ret;
    }

    slen = sizeof(mem_max);
    ret = amdgpu_query_info(device, AMDGPU_INFO_MEMORY, slen, &mem_max);

    if (ret < 0) {
        return ret;
    }

    amdgpu_info->vram_total = get_mib(mem_max);
    amdgpu_info->vram = get_mib(mem);

    if (amdgpu_info->vram > amdgpu_info->vram_max) {
        amdgpu_info->vram_max = amdgpu_info->vram;
    }

    return 0;
}

int get_amdgpu_power(amdgpu_device_handle device, struct amdgpu_info *amdgpu_info) {
    size_t len;

    len = sizeof(amdgpu_info->power);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_AVG_POWER, len, &amdgpu_info->power);

    if (ret < 0) {
        return ret;
    }

    if (amdgpu_info->power > amdgpu_info->power_max) {
        amdgpu_info->power_max = amdgpu_info->power;
    }

    return 0;
}

int get_amdgpu_temp(amdgpu_device_handle device, struct amdgpu_info *amdgpu_info) {
    size_t len;

    len = sizeof(amdgpu_info->temp);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_TEMP, len, &amdgpu_info->temp);

    if (ret < 0) {
        return ret;
    }

    amdgpu_info->temp /= 1000;

    if (amdgpu_info->temp > amdgpu_info->temp_max) {
        amdgpu_info->temp_max = amdgpu_info->temp;
    }

    return 0;
}

int get_amdgpu_freq(amdgpu_device_handle device, struct amdgpu_info *amdgpu_info) {
    size_t len;

    len = sizeof(amdgpu_info->freq);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GFX_SCLK, len, &amdgpu_info->freq);

    if (ret < 0) {
        return ret;
    }

    if (amdgpu_info->freq > amdgpu_info->freq_max) {
        amdgpu_info->freq_max = amdgpu_info->freq;
    }

    return 0;
}

int get_amdgpu_name(amdgpu_device_handle device, struct amdgpu_info *amdgpu_info) {

    const char *name = amdgpu_get_marketing_name(device);

    if (name == NULL) {
        return -1;
    }

    strlcpy(amdgpu_info->name, name, 64);

    return 0;
}

void amdgpu_init(int fd) {

    uint32_t major_version;
    uint32_t minor_version;

    amdgpu_device_handle device;

    int ret = amdgpu_device_initialize(fd, &major_version, &minor_version, &device);

    if (ret < 0) {
        return;
    }

    print_title("gpu (amdgpu)");
    if (get_amdgpu_name(device, &amdgpu_info) == 0) {
        pretty_print("GPU name", amdgpu_info.name);
        printf("\n");
    }

    if (get_amdgpu_usage(device, &amdgpu_info) == 0) {
        print_progress("GPU usage", amdgpu_info.usage, 100);
        printf(" (%d%%)\n", amdgpu_info.usage);

        print_progress("Max GPU usage", amdgpu_info.usage_max, 100);
        printf(" (%d%%)\n", amdgpu_info.usage_max);
        printf("\n");
    }

    if (get_amdgpu_mem_usage(device, &amdgpu_info) == 0) {
        print_progress("VRAM usage", amdgpu_info.vram, amdgpu_info.vram_total);
        printf(" (%dMiB / %dMiB)\n", amdgpu_info.vram, amdgpu_info.vram_total);

        print_progress("Max VRAM usage", amdgpu_info.vram_max, amdgpu_info.vram_total);
        printf(" (%dMiB / %dMiB)\n", amdgpu_info.vram_max, amdgpu_info.vram_total);
        printf("\n");
    }

    if (get_amdgpu_power(device, &amdgpu_info) == 0) {
        char power_buff[32];

        snprintf(power_buff, 32, "%d W", amdgpu_info.power);
        pretty_print("GPU power", power_buff);

        snprintf(power_buff, 32, "%d W", amdgpu_info.power_max);
        pretty_print("Max GPU power", power_buff);
        printf("\n");
    }

    if (get_amdgpu_freq(device, &amdgpu_info) == 0) {
        char freq_buff[32];

        snprintf(freq_buff, 32, "%d MHz", amdgpu_info.freq);
        pretty_print("GPU freq", freq_buff);

        snprintf(freq_buff, 32, "%d MHz", amdgpu_info.freq_max);
        pretty_print("Max GPU freq", freq_buff);
        printf("\n");
    }

    if (get_amdgpu_temp(device, &amdgpu_info) == 0) {
        char temp_buff[32];

        snprintf(temp_buff, 32, "%d°C", amdgpu_info.temp);
        pretty_print("GPU temp", temp_buff);

        snprintf(temp_buff, 32, "%d°C", amdgpu_info.temp_max);
        pretty_print("Max GPU temp", temp_buff);
        printf("\n");
    }
    
    amdgpu_device_deinitialize(device);
}
