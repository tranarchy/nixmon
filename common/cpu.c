#include <stdlib.h>
#include <unistd.h>

#include "../include/info.h"

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

void get_cpu_usage_percent(struct cpu_info *cpu, long long *cpu_usage, long long *cpu_usage_prev, int cpu_states_num, int cpu_idle_state) {
    long long cpu_usage_sum = 0;
    long long cpu_non_idle_usage_sum = 0;
    
    for (int i = 0; i < cpu_states_num; i++) {
        cpu_usage_sum += cpu_usage[i] - cpu_usage_prev[i];
    }

    long long cpu_idle = cpu_usage[cpu_idle_state] - cpu_usage_prev[cpu_idle_state];
    cpu_non_idle_usage_sum = cpu_usage_sum - cpu_idle;

    cpu->usage = 100.0 * cpu_non_idle_usage_sum / cpu_usage_sum;
        
    for (int i = 0; i < cpu_states_num; i++) {
        cpu_usage_prev[i] = cpu_usage[i];
    }

    if (cpu->usage > cpu->usage_max) {
        cpu->usage_max =  cpu->usage;
    }
}
