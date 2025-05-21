OUTPUT = nixmon
INSTALL_PATH = /usr/local/bin

SOURCE_FILES = main.c gen.c cpu.c mem.c storage.c util/util.c gpu/amdgpu.c gpu/gpu.c

CFLAGS = -Wall -Wextra -Wpedantic -I/usr/include/libdrm -I/usr/X11R6/include -I/usr/X11R6/include/libdrm -L/usr/X11R6/lib
LIBS = -ldrm_amdgpu -ldrm

all:
	cc $(SOURCE_FILES) -o $(OUTPUT) $(CFLAGS) $(LIBS)

install:
	cp -f $(OUTPUT) $(INSTALL_PATH)

clean:
	rm -f $(OUTPUT)