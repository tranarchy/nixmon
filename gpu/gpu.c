#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "xf86drm.h"

#include "../include/info.h"
#include "../include/gpu.h"

int get_gpu_fd(void) {
    int fd = open("/dev/dri/renderD128", O_RDONLY);

    if (fd == -1) {
        return -1;
    }

    return fd;
}

char *get_driver(void) {
    int fd = get_gpu_fd();
    drmVersionPtr drmVersion = drmGetVersion(fd);
    close(fd);

    if (drmVersion == NULL) {
        return NULL;
    }

    return drmVersion->name;
}


void gpu_init(struct gpu_info *gpu) {
    int fd;

    char *driver = get_driver();

    if (driver == NULL) {
        return;
    }

    fd = get_gpu_fd();

    if (strcmp(driver, "amdgpu") == 0) {
        amdgpu_init(fd, gpu);
    }

    close(fd);
}
