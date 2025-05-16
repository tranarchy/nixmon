void gen_init();
void cpu_init();
void mem_init();
void storage_init();

#if defined(__linux__)
    void i915_init();
    void amdgpu_init();
#endif