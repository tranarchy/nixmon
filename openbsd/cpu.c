#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/sensors.h>

#include "../include/info.h"
#include "../include/cpu.h"

#define CPU_IDLE_STATE 5 
#define CPU_STATES_NUM 6

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

static int microkelvin_to_celsius(long int value) {
    return (value - 273150000) / 1E6;
}

static struct sensor get_sensor(char *sensor_name, int sensor_type) {
    int ret;
    size_t len, slen;
    struct sensor sensor;
    struct sensordev sensordev;

    int mib[5];

    mib[0] = CTL_HW;
    mib[1] = HW_SENSORS;

    for (int i = 0; i < 1024; i++) {

        mib[2] = i;

        len = sizeof(struct sensordev);
        ret = sysctl(mib, 3, &sensordev, &len, NULL, 0);

        if (ret == -1) {
            continue;
        }

        mib[3] = sensor_type;

        for (int j = 0; j < sensordev.maxnumt[sensor_type]; j++) {
            mib[4] = j;
            slen = sizeof(struct sensor);
            ret = sysctl(mib, 5, &sensor, &slen, NULL, 0);

            if (ret == -1) {
                continue;
            }

            if (strcmp(sensordev.xname, sensor_name) == 0) {
                return sensor;
            }

        }
    }

    sensor.value = -1;

    return sensor;
}

int get_cpu_usage(struct cpu_info *cpu) {
    int mib[2];

    mib[0] = CTL_KERN;
    mib[1] = KERN_CPTIME;

    size_t len = sizeof(cpu_usage);
    int ret = sysctl(mib, 2, &cpu_usage, &len, NULL, 0);

    if (ret == -1) {
        return ret;
    }

    get_cpu_usage_percent(cpu, cpu_usage, cpu_usage_prev, CPU_STATES_NUM, CPU_IDLE_STATE);

    return 0;
}

int get_cpu_freq(struct cpu_info *cpu) {
    struct sensor freq_sensor = get_sensor("cpu0", SENSOR_FREQ);

    if (freq_sensor.value == -1) {
        return -1;
    }

    cpu->freq = freq_sensor.value / 1E15; // μHz

    if (cpu->freq > cpu->freq_max) {
        cpu->freq_max = cpu->freq;
    }

    return 0;
}

int get_cpu_temp(struct cpu_info *cpu) {
    
    struct sensor temp_sensor = get_sensor("cpu0", SENSOR_TEMP);

     if (temp_sensor.value == -1) {
        return -1;
    }

    cpu->temp = microkelvin_to_celsius(temp_sensor.value);

    if (cpu->temp > cpu->temp_max) {
        cpu->temp_max = cpu->temp;
    }

    return 0;
}
