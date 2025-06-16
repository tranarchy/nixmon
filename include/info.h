struct gen_info {
    int uptime_day;
    int uptime_hour;
    int uptime_min;
    int uptime_sec;
};

struct cpu_info {
    long threads;
    long threads_online;

    int usage;
    int usage_max;

    double freq;
    double freq_max;

    #ifdef __linux__
        double freq_total;
    #endif

    int temp;
    int temp_max;

    double loadavg[3];
};

struct mem_info {
    int total;

    int used;
    int used_max;
};

struct storage_info {
    char mount_point[16];
    char size_suffix[4];

    int total;
    int used;
};

#ifndef __APPLE__
    struct gpu_info {
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
#endif
