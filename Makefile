OUTPUT = nixmon
INSTALL_PATH = /usr/local/bin

SOURCE_FILES = main.c gen.c cpu.c mem.c storage.c util/util.c
SOURCE_FILES_MACOS = apple/cpu_temp.m
SOURCE_FILES_GPU = gpu/amdgpu.c gpu/gpu.c

CFLAGS = -Wall -Wextra -Wpedantic
CFLAGS_MACOS = -framework IOKit -framework foundation
CFLAGS_LIBS = -I/usr/include/libdrm -I/usr/X11R6/include -I/usr/X11R6/include/libdrm -L/usr/X11R6/lib
LIBS = -ldrm_amdgpu -ldrm

main:
	cc $(SOURCE_FILES) $(SOURCE_FILES_GPU) -o $(OUTPUT) $(CFLAGS) $(CFLAGS_LIBS) $(LIBS)

macos:
	cc $(SOURCE_FILES) $(SOURCE_FILES_MACOS) -o $(OUTPUT) $(CFLAGS) $(CFLAGS_MACOS)

install:
	cp -f $(OUTPUT) $(INSTALL_PATH)

clean:
	rm -f $(OUTPUT)