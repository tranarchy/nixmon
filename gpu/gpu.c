#include <fcntl.h>
#include <unistd.h>

#include "gpu.h"

int gpu_init(void) {
    int fd;

    fd = open("/dev/dri/renderD128", O_RDWR);

    if (fd == -1) {
        return fd;
    }

    amdgpu_init(fd);

    close(fd);

    return 0;
}
