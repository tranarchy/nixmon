OUTPUT = nixmon
INSTALL_PATH = /usr/local/bin

MAIN_SOURCE_FILES = main.c gen.c cpu.c mem.c storage.c util/util.c
COMMON_SOURCE_FILES =  common/gen.c common/cpu.c common/storage.c
COMMON_BSD_SOURCE_FILES = common_bsd/storage.c common_bsd/mem.c

LINUX_SOURCES_FILES = linux/cpu.c linux/mem.c linux/storage.c
OPENBSD_SOURCE_FILES = openbsd/cpu.c
FREEBSD_SOURCE_FILES = freebsd/cpu.c
NETBSD_SOURCE_FILES = netbsd/cpu.c
MACOS_SOURCE_FILES = apple/cpu.c apple/mem.c common_bsd/storage.c apple/cpu_temp.m

GPU_SOURCE_FILES = gpu/amdgpu.c gpu/gpu.c

LINUX_CFLAG_LIBS = -I/usr/include/libdrm
OPENBSD_CLFAG_LIBS = -I/usr/X11R6/include -I/usr/X11R6/include/libdrm -L/usr/X11R6/lib
FREEBSD_CFLAG_LIBS = -I/usr/local/include -I/usr/local/include/libdrm -L/usr/local/lib
NETBSD_CLFAG_LIBS = -I/usr/X11R7/include -I/usr/X11R7/include/libdrm -L/usr/X11R7/lib

CFLAGS = -Wall -Wextra -Wpedantic
MACOS_CFLAGS = -framework IOKit -framework foundation

LIBS = -ldrm_amdgpu -ldrm

main:
	cc $(MAIN_SOURCE_FILES) $(COMMON_SOURCE_FILES) $(LINUX_SOURCES_FILES) $(GPU_SOURCE_FILES) -o $(OUTPUT) $(CFLAGS) $(LINUX_CFLAG_LIBS) $(LIBS)

obsd:
	cc $(MAIN_SOURCE_FILES) $(COMMON_SOURCE_FILES) $(COMMON_BSD_SOURCE_FILES) $(OPENBSD_SOURCE_FILES) $(GPU_SOURCE_FILES) -o $(OUTPUT) $(CFLAGS) $(OPENBSD_CLFAG_LIBS) $(LIBS)

fbsd:
	cc $(MAIN_SOURCE_FILES) $(COMMON_SOURCE_FILES) $(COMMON_BSD_SOURCE_FILES) $(FREEBSD_SOURCE_FILES) $(GPU_SOURCE_FILES) -o $(OUTPUT) $(CFLAGS) $(FREEBSD_CFLAG_LIBS) $(LIBS)

nbsd:
	cc $(MAIN_SOURCE_FILES) $(COMMON_SOURCE_FILES) $(COMMON_BSD_SOURCE_FILES) $(NETBSD_SOURCE_FILES) $(GPU_SOURCE_FILES) -o $(OUTPUT) $(CFLAGS) $(NETBSD_CLFAG_LIBS) $(LIBS)

macos:
	cc $(MAIN_SOURCE_FILES) $(COMMON_SOURCE_FILES) $(MACOS_SOURCE_FILES) -o $(OUTPUT) $(CFLAGS) $(MACOS_CFLAGS)

install:
	cp -f $(OUTPUT) $(INSTALL_PATH)

clean:
	rm -f $(OUTPUT)