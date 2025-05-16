OUTPUT = nixmon
INSTALL_PATH = /usr/local/bin

SOURCE_FILES = main.c gen.c cpu.c mem.c storage.c util/util.c

ifeq ($(shell uname), Linux)
	SOURCE_FILES += i915.c amdgpu.c
endif

all:
	cc -Wall $(SOURCE_FILES) -o $(OUTPUT)

install:
	cp -f $(OUTPUT) $(INSTALL_PATH)

clean:
	rm -f $(OUTPUT)