#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/sysctl.h>

#include "../include/info.h"
#include "../include/cpu.h"

#define CPU_IDLE_STATE 4
#define CPU_STATES_NUM 5

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

int get_cpu_usage(struct cpu_info *cpu) {
    int mib[2];

    mib[0] = CTL_KERN;
    mib[1] = KERN_CP_TIME;

    size_t len = sizeof(cpu_usage);
    int ret = sysctl(mib, 2, &cpu_usage, &len, NULL, 0);

    if (ret == -1) {
        return ret;
    }

    get_cpu_usage_percent(cpu, cpu_usage, cpu_usage_prev, CPU_STATES_NUM, CPU_IDLE_STATE);

    return 0;
}

int get_cpu_freq(struct cpu_info *cpu) {
    size_t len;
    int raw_freq;

    len = sizeof(raw_freq);
    int ret = sysctlbyname("machdep.cpu.frequency.current", &raw_freq, &len, NULL, 0);

    if (ret == -1) {
        return ret;
    }

    cpu->freq = raw_freq / 1000;

    if (cpu->freq > cpu->freq_max) {
        cpu->freq_max = cpu->freq;
    }

    return 0;
}

int get_cpu_temp(struct cpu_info *cpu) {
    // todo
    return -1;
}
