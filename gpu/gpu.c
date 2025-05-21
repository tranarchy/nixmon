#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "gpu.h"

char *driver = NULL;

int get_gpu_fd(void) {
    int fd = open("/dev/dri/renderD128", O_RDONLY);

    if (fd == -1) {
        return -1;
    }

    return fd;
}


void gpu_init(void) {

    int fd = get_gpu_fd();
    amdgpu_init(fd);        
    close(fd);

}
