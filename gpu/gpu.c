#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "xf86drm.h"

#include "gpu.h"

char *driver = NULL;

int get_gpu_fd(void) {
    int fd = open("/dev/dri/renderD128", O_RDONLY);

    if (fd == -1) {
        return -1;
    }

    return fd;
}

void get_driver(void) {
    int fd = get_gpu_fd();


    if (fd == -1) {
        return;
    }

    drmVersionPtr drmVersion = drmGetVersion(fd);
    driver = drmVersion->name;

    close(fd);
}

void gpu_init(void) {
    if (driver == NULL) {
        get_driver();
    } 
    
    int fd = get_gpu_fd();
    if (strcmp(driver, "amdgpu") >= 0) {
        amdgpu_init(fd);        
    }
    close(fd);

}
