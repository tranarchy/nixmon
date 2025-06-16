#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "xf86drm.h"

#include "../info.h"

#include "gpu.h"


char *driver = NULL;
int no_driver = 0;

int get_gpu_fd(void) {
    int fd = open("/dev/dri/renderD128", O_RDONLY);

    if (fd == -1) {
        return -1;
    }

    return fd;
}

void get_driver(void) {
    int fd = get_gpu_fd();
    drmVersionPtr drmVersion = drmGetVersion(fd);
    close(fd);

    if (drmVersion == NULL) {
        no_driver = 1;

        return;
    }

    driver = drmVersion->name;
}


void gpu_init(struct gpu_info *gpu) {
    int fd;

    if (driver == NULL) {
        if (!no_driver) {
            get_driver();
        } else {
            return;
        }
    }

    if (strcmp(driver, "amdgpu") == 0) {
        fd = get_gpu_fd();
        amdgpu_init(fd, gpu);        
        close(fd);
    }

}
