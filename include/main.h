void gen_init(struct gen_info *gen);
void cpu_init(struct cpu_info *cpu);
void mem_init(struct mem_info *mem);
void storage_init(struct storage_info *storage);

#ifndef __APPLE__
    void gpu_init(struct gpu_info *gpu_info);
#endif
