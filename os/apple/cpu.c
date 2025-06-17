#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <mach/mach.h>

#include <sys/sysctl.h>

#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include "../../include/info.h"
#include "../../include/cpu.h"

#define CPU_IDLE_STATE 2
#define CPU_STATES_NUM 4

long long cpu_usage[CPU_STATES_NUM];
long long cpu_usage_prev[CPU_STATES_NUM] = { 0 };

int get_cpu_usage(struct cpu_info *cpu) {
    natural_t proc_count;
    mach_msg_type_number_t len = HOST_VM_INFO64_COUNT;
    processor_cpu_load_info_t cpu_load_info;

    int ret = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &proc_count, (processor_info_array_t *)&cpu_load_info, &len);

    if (ret == -1) {
        return ret;
    }

    for (int i = 0; i < CPU_STATES_NUM; i++) {
        cpu_usage[i] = 0;
        for (unsigned int j = 0; j < proc_count; j++) {
            cpu_usage[i] += cpu_load_info[j].cpu_ticks[i];
        }
    }

    get_cpu_usage_percent(cpu, cpu_usage, cpu_usage_prev, CPU_STATES_NUM, CPU_IDLE_STATE);

    return 0;
}

int get_cpu_freq(struct cpu_info *cpu) {
    unsigned int freq_raw;

    CFDictionaryRef matching = IOServiceMatching("AppleARMIODevice");

    io_iterator_t iter;
    io_registry_entry_t entry;
    IOServiceGetMatchingServices(kIOMainPortDefault, matching, &iter);

    while ((entry = IOIteratorNext(iter)) != 0) {
        io_name_t name;

        IORegistryEntryGetName(entry, name);

        if (strcmp(name, "pmgr") == 0) {
            break;
        };
    }

    CFTypeRef pCoreRef = IORegistryEntryCreateCFProperty(entry, CFSTR("voltage-states5-sram"), kCFAllocatorDefault, 0);

    size_t length = CFDataGetLength(pCoreRef);

    CFDataGetBytes(pCoreRef, CFRangeMake(length - 8, 4), (UInt8 *) &freq_raw);

    cpu->freq = freq_raw / 1000.0 / 1000.0 / 1000.0;

    if (cpu->freq > cpu->freq_max) {
        cpu->freq_max = cpu->freq;
    }

    return 0;
}

int get_cpu_temp(struct cpu_info *cpu) {
    cpu->temp = get_macos_cpu_temp(0xff00, 5);

    if (cpu->temp > cpu->temp_max) {
        cpu->temp_max = cpu->temp;
    }

    return 0;
}
