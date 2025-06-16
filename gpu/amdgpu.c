#include <stdio.h>
#include <string.h>

#include <libdrm/amdgpu.h>
#include <libdrm/amdgpu_drm.h>

#include "../info.h"

#include "../util/util.h"

int get_amdgpu_usage(amdgpu_device_handle device, struct gpu_info *gpu) {
    size_t len;

    len = sizeof(gpu->usage);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_LOAD, len, &gpu->usage);

    if (ret < 0) {
        return ret;
    }

    if (gpu->usage > gpu->usage_max) {
        gpu->usage_max = gpu->usage;
    }

    return 0;
}

int get_amdgpu_mem_usage(amdgpu_device_handle device, struct gpu_info *gpu) {
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

    gpu->vram_total = get_mib(mem_max);
    gpu->vram = get_mib(mem);

    if (gpu->vram > gpu->vram_max) {
        gpu->vram_max = gpu->vram;
    }

    return 0;
}

int get_amdgpu_power(amdgpu_device_handle device, struct gpu_info *gpu) {
    size_t len;

    len = sizeof(gpu->power);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_AVG_POWER, len, &gpu->power);

    if (ret < 0) {
        return ret;
    }

    if (gpu->power > gpu->power_max) {
        gpu->power_max = gpu->power;
    }

    return 0;
}

int get_amdgpu_temp(amdgpu_device_handle device, struct gpu_info *gpu) {
    size_t len;

    len = sizeof(gpu->temp);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GPU_TEMP, len, &gpu->temp);

    if (ret < 0) {
        return ret;
    }

    gpu->temp /= 1000;

    if (gpu->temp > gpu->temp_max) {
        gpu->temp_max = gpu->temp;
    }

    return 0;
}

int get_amdgpu_freq(amdgpu_device_handle device, struct gpu_info *gpu) {
    size_t len;

    len = sizeof(gpu->freq);
    int ret = amdgpu_query_sensor_info(device, AMDGPU_INFO_SENSOR_GFX_SCLK, len, &gpu->freq);

    if (ret < 0) {
        return ret;
    }

    if (gpu->freq > gpu->freq_max) {
        gpu->freq_max = gpu->freq;
    }

    return 0;
}

int get_amdgpu_name(amdgpu_device_handle device, struct gpu_info *gpu) {

    const char *name = amdgpu_get_marketing_name(device);

    if (name == NULL) {
        return -1;
    }

    strlcpy(gpu->name, name, 64);

    return 0;
}

void amdgpu_init(int fd, struct gpu_info *gpu) {

    uint32_t major_version;
    uint32_t minor_version;

    amdgpu_device_handle device;

    int ret = amdgpu_device_initialize(fd, &major_version, &minor_version, &device);

    if (ret < 0) {
        return;
    }

    print_title("gpu (amdgpu)");
    if (get_amdgpu_name(device, gpu) == 0) {
        pretty_print("GPU name", gpu->name);
        printf("\n");
    }

    if (get_amdgpu_usage(device, gpu) == 0) {
        print_progress("GPU usage", gpu->usage, 100);
        printf(" (%d%%)\n", gpu->usage);

        print_progress("Max GPU usage", gpu->usage_max, 100);
        printf(" (%d%%)\n", gpu->usage_max);
        printf("\n");
    }

    if (get_amdgpu_mem_usage(device, gpu) == 0) {
        print_progress("VRAM usage", gpu->vram, gpu->vram_total);
        printf(" (%dMiB / %dMiB)\n", gpu->vram, gpu->vram_total);

        print_progress("Max VRAM usage", gpu->vram_max, gpu->vram_total);
        printf(" (%dMiB / %dMiB)\n", gpu->vram_max, gpu->vram_total);
        printf("\n");
    }

    if (get_amdgpu_power(device, gpu) == 0) {
        char power_buff[32];

        snprintf(power_buff, 32, "%d W", gpu->power);
        pretty_print("GPU power", power_buff);

        snprintf(power_buff, 32, "%d W", gpu->power_max);
        pretty_print("Max GPU power", power_buff);
        printf("\n");
    }

    if (get_amdgpu_freq(device, gpu) == 0) {
        char freq_buff[32];

        snprintf(freq_buff, 32, "%d MHz", gpu->freq);
        pretty_print("GPU freq", freq_buff);

        snprintf(freq_buff, 32, "%d MHz", gpu->freq_max);
        pretty_print("Max GPU freq", freq_buff);
        printf("\n");
    }

    if (get_amdgpu_temp(device, gpu) == 0) {
        char temp_buff[32];

        snprintf(temp_buff, 32, "%d°C", gpu->temp);
        pretty_print("GPU temp", temp_buff);

        snprintf(temp_buff, 32, "%d°C", gpu->temp_max);
        pretty_print("Max GPU temp", temp_buff);
        printf("\n");
    }
    
    amdgpu_device_deinitialize(device);
}
