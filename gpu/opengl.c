#include <stdio.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "../util/util.h"

struct opengl_info {
    char *name;
    char *vendor;

    GLint vram_total;

    GLint vram;
    GLint vram_max;
};

struct opengl_info opengl_info;

// NVX extension for VRAM usage
int get_opengl_mem_usage(struct opengl_info *opengl_info) {
    GLint vram_available;
    
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &opengl_info->vram_total);
    
    if (opengl_info->vram_total == GLFW_FALSE) {
        return -1;
    }

    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &vram_available);

    if (vram_available == GLFW_FALSE) {
        return -1;
    }

    opengl_info->vram = (opengl_info->vram_total - vram_available) / 1024;
    opengl_info->vram_total /= 1024;

    if (opengl_info->vram > opengl_info->vram_max) {
        opengl_info->vram_max = opengl_info->vram;
    }

    return 0;
}

int get_opengl_gpu_name(struct opengl_info *opengl_info) {
    opengl_info->name = (char*)glGetString(GL_RENDERER);

    if (opengl_info->name == GLFW_FALSE) {
        return -1;
    }

    return 0;
}

int get_opengl_vendor(struct opengl_info *opengl_info) {
    opengl_info->vendor = (char*)glGetString(GL_VENDOR);

    if (opengl_info->vendor == GLFW_FALSE) {
        return -1;
    }

    return 0;
}

void opengl_init(void) {
    GLFWwindow *window;
    int ret;

    char title_buff[64];

    ret = glfwInit();

    if (ret == GLFW_FALSE) {
        return;
    }

    window = glfwCreateWindow(320, 200, "a", NULL, NULL);
    glfwMakeContextCurrent(window);

    get_opengl_vendor(&opengl_info);

    snprintf(title_buff, 64, "gpu (%s)", opengl_info.vendor);

    print_title(title_buff);
    if (get_opengl_gpu_name(&opengl_info) == 0) {
        pretty_print("GPU name", opengl_info.name);
        printf("\n");
    }

    if (get_opengl_mem_usage(&opengl_info) == 0) {
        print_progress("VRAM usage", opengl_info.vram, opengl_info.vram_total);
        printf(" (%dMiB / %dMiB)\n", opengl_info.vram, opengl_info.vram_total);

        print_progress("Max VRAM usage", opengl_info.vram_max, opengl_info.vram_total);
        printf(" (%dMiB / %dMiB)\n", opengl_info.vram_max, opengl_info.vram_total);
        printf("\n");
    }


    glfwDestroyWindow(window);
    glfwTerminate();
}