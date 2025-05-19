#include <fcntl.h>
#include <unistd.h>

#include "gpu.h"

void gpu_init(void) {
    int fd;

    fd = open("/dev/dri/renderD128", O_RDONLY);

    if (fd == -1) {
        return;
    }

    amdgpu_init(fd);

    close(fd);
}
