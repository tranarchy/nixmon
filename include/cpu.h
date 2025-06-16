int get_loadavg(struct cpu_info *cpu);
int get_threads(struct cpu_info *cpu);
int get_cpu_usage(struct cpu_info *cpu);
void get_cpu_usage_percent(struct cpu_info *cpu, long long *cpu_usage, long long *cpu_usage_prev, int cpu_states_num, int cpu_idle_state);
int get_cpu_freq(struct cpu_info *cpu);
int get_cpu_temp(struct cpu_info *cpu);

#ifdef __APPLE__
    double get_macos_cpu_temp(int page, int usage);
#endif
